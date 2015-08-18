/**
 * @file upload_manager_impl.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "upload_manager_impl.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/uploader/error.hpp"
#include "mediafire_sdk/utils/mutex.hpp"

namespace action_token = mf::api::user::get_action_token;
namespace posix_time = boost::posix_time;
using sclock = std::chrono::steady_clock;

namespace
{
const std::chrono::seconds kActionTokenRetry(15);
// Default lifetime is 1440 minutes(24h).  We should get a new upload token
// before it expires.
const std::chrono::minutes kActionTokenLife(1440 / 4 * 3);

mf::uploader::detail::UploadHandle NextUploadHandle()
{
    static mf::uploader::detail::UploadHandle upload_handle = {0};
    static mf::utils::mutex mutex;

    mf::utils::lock_guard<mf::utils::mutex> lock(mutex);
    ++upload_handle.id;
    return upload_handle;
}
}  // namespace

namespace mf
{
namespace uploader
{
namespace detail
{

UploadManagerImpl::UploadManagerImpl(
        ::mf::api::SessionMaintainer * session_maintainer)
        : session_maintainer_(session_maintainer),
          io_service_(session_maintainer->HttpConfig()->GetWorkIoService()),
          action_token_state_(ActionTokenState::Invalid),
          action_token_retry_timer_(*io_service_),
          max_concurrent_hashings_(2),
          max_concurrent_uploads_(2),
          current_hashings_(0),
          current_uploads_(0),
          disable_enqueue_(false)
{
}

UploadManagerImpl::~UploadManagerImpl()
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    disable_enqueue_ = true;

    std::set<StateMachinePointer> requests;
    std::swap(requests, requests_);

    // Unlock before calling external
    lock.unlock();

    for (auto & request : requests)
    {
        request->Disconnect();
        request->process_event(
                event::Error{make_error_code(mf::uploader::errc::Cancelled),
                             "Cancelled due to shutdown."});
    }
}

UploadHandle UploadManagerImpl::Add(const UploadRequest & upload_request,
                                    StatusCallback callback)
{
    auto upload_handle = NextUploadHandle();
    UploadConfig config;

    config.upload_handle = upload_handle;
    config.session_maintainer = session_maintainer_;
    config.filepath = upload_request.local_file_path_;
    config.on_duplicate_action = upload_request.on_duplicate_action_;
    config.callback_interface
            = static_cast<UploadStateMachineCallbackInterface *>(this);
    config.status_callback = callback;
    config.cloud_file_name = upload_request.utf8_target_name_;
    config.target_folder = upload_request.upload_target_folder_;

    auto request = std::make_shared<UploadStateMachine>(std::move(config));

    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        requests_.insert(request);
    }

    // Initialization step
    request->process_event(event::Start{});

    Tick();

    return upload_handle;
}

void UploadManagerImpl::ModifyUpload(
        UploadHandle upload_handle,
        ::mf::uploader::UploadModification upload_modification)
{
    class Visitor : public boost::static_visitor<>
    {
    public:
        Visitor(UploadManagerImpl * um, UploadHandle upload_handle)
                : this_(um), upload_handle_(upload_handle)
        {
        }

        void operator()(modification::Cancel) const
        {
            mf::utils::unique_lock<mf::utils::mutex> lock(this_->mutex_);
            for (auto & request : this_->requests_)
            {
                if (request->Handle().id == upload_handle_.id)
                {
                    lock.unlock();
                    request->ProcessEvent(event::Error{
                            make_error_code(mf::uploader::errc::Cancelled),
                            "Cancellation requested"});
                    return;
                }
            }
        }

        void operator()(modification::Pause) const
        {
            mf::utils::unique_lock<mf::utils::mutex> lock(this_->mutex_);
            for (auto & request : this_->requests_)
            {
                if (request->Handle().id == upload_handle_.id)
                {
                    lock.unlock();
                    request->ProcessEvent(event::Error{
                            make_error_code(mf::uploader::errc::Paused),
                            "Pause requested."});
                    return;
                }
            }
        }

    private:
        UploadManagerImpl * this_;
        const UploadHandle & upload_handle_;
    };

    boost::apply_visitor(Visitor(this, upload_handle), upload_modification);
}

void UploadManagerImpl::Tick()
{
    Tick_StartUploads();

    Tick_StartHashings();
}

void UploadManagerImpl::EnqueueTick()
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    if (!disable_enqueue_)
    {
        io_service_->post(
                boost::bind(&UploadManagerImpl::Tick, shared_from_this()));
    }
}

void UploadManagerImpl::Tick_StartHashings()
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    while (!to_hash_.empty()
           && (current_hashings_ + enqueued_to_start_hashings_.size())
                      < max_concurrent_hashings_)
    {
        auto request = to_hash_.front();
        to_hash_.pop_front();

        enqueued_to_start_hashings_.insert(request.get());

        auto self = shared_from_this();

        // Enqueue start
        io_service_->post([this, self, request]()
                          {
                              request->process_event(event::StartHash{});
                          });
    }
}

void UploadManagerImpl::Tick_StartUploads()
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

#ifndef NDEBUG
    // Ensure calling process event never causes recursion here.
    static bool recursing = false;
    assert(!recursing);
    recursing = true;
#endif

    auto CanUploadMore = [this]()
    {
        auto size = current_uploads_ + enqueued_to_start_uploads_.size();
        auto result = (size < max_concurrent_uploads_);

        return result;
    };

    if (!to_upload_.empty())
    {
        const auto now = sclock::now();
        if (action_token_state_ != ActionTokenState::Valid
            || action_token_expires_ < now)
        {
            // Invalid token

            if (action_token_state_ == ActionTokenState::Error
                && now < action_token_expires_)
            {
                // Skip till timeout.
            }
            else
            {
                action_token_state_ = ActionTokenState::Retrieving;

                // Unlock before calling external
                lock.unlock();

                auto self = shared_from_this();
                session_maintainer_->Call(
                        action_token::Request(action_token::Type::Upload),
                        [self](action_token::Response response)
                        {
                            self->HandleActionToken(response);
                        });
            }
            // If retrieving or error and not reached retry timeout, skip.
        }
        else if (CanUploadMore())
        {
            assert(!action_token_.empty());

            const auto token = action_token_;

            for (auto it = to_upload_.begin();
                 it != to_upload_.end() && CanUploadMore();)
            {
                // Skip duplicate hashes
                auto request = *it;
                if (uploading_hashes_.find(request->hash())
                    == uploading_hashes_.end())
                {
                    // Remove iterator before process event.
                    it = to_upload_.erase(it);

                    uploading_hashes_.insert(request->hash());

                    enqueued_to_start_uploads_.insert(request.get());

                    auto self = shared_from_this();
                    io_service_->post([this, self, request, token]()
                                      {
                                          request->process_event(
                                                  event::StartUpload{token});
                                      });

                    // Only do one
                    break;
                }
                else
                {
                    ++it;
                }
            }
        }
    }

#ifndef NDEBUG
    if (!lock)
        lock.lock();
    recursing = false;
#endif
}

void UploadManagerImpl::HandleActionToken(
        const mf::api::user::get_action_token::Response & response)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    if (!response.response_data)
    {
        // Retry soon if unable to obtain token.
        action_token_state_ = ActionTokenState::Error;
        action_token_expires_ = sclock::now() + kActionTokenRetry;
        action_token_retry_timer_.expires_at(action_token_expires_);

        // unlock before calling external
        lock.unlock();

        action_token_retry_timer_.async_wait(boost::bind(
                &UploadManagerImpl::ActionTokenRetry, shared_from_this(),
                boost::asio::placeholders::error));
    }
    else
    {
        const auto & response_data = *response.response_data;

        action_token_state_ = ActionTokenState::Valid;
        action_token_ = response_data.action_token;
        action_token_expires_ = sclock::now() + kActionTokenLife;

        // Unlock before calling external
        lock.unlock();

        Tick();
    }
}

void UploadManagerImpl::ActionTokenRetry(const boost::system::error_code & err)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    if (!err && action_token_state_ == ActionTokenState::Error)
    {
        action_token_.clear();
        action_token_state_ = ActionTokenState::Invalid;

        // Unlock before calling external
        lock.unlock();

        Tick();
    }
}

// -- UploadStateMachineCallbackInterface --------------------------------------
void UploadManagerImpl::HandleAddToHash(StateMachinePointer request)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    to_hash_.push_back(request);

    // Unlock before calling external
    lock.unlock();

    EnqueueTick();
}
void UploadManagerImpl::HandleRemoveToHash(StateMachinePointer request)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    auto it = std::find(to_hash_.begin(), to_hash_.end(), request);
    if (it != to_hash_.end())
        to_hash_.erase(it);

    // Unlock before calling external
    lock.unlock();

    EnqueueTick();
}

void UploadManagerImpl::HandleAddToUpload(StateMachinePointer request)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    to_upload_.push_back(request);

    // Unlock before calling external
    lock.unlock();

    EnqueueTick();
}
void UploadManagerImpl::HandleRemoveToUpload(StateMachinePointer request)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    auto it = std::find(to_upload_.begin(), to_upload_.end(), request);
    if (it != to_upload_.end())
        to_upload_.erase(it);

    // Unlock before calling external
    lock.unlock();

    EnqueueTick();
}

void UploadManagerImpl::HandleComplete(StateMachinePointer request)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    // Clear
    enqueued_to_start_hashings_.erase(request.get());
    enqueued_to_start_uploads_.erase(request.get());

    requests_.erase(request);

    const auto chunk_data = request->GetChunkData();

    // Remove hashes to allow duplicates
    if (!chunk_data.hash.empty())
    {
        uploading_hashes_.erase(request->hash());
    }

    // Unlock before calling external
    lock.unlock();

    EnqueueTick();
}

void UploadManagerImpl::IncrementHashingCount(StateMachinePointer request)
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    ++current_hashings_;
    enqueued_to_start_hashings_.erase(request.get());
}

void UploadManagerImpl::DecrementHashingCount(StateMachinePointer)
{
    EnqueueTick();

    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    --current_hashings_;
}

void UploadManagerImpl::IncrementUploadingCount(StateMachinePointer request)
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    ++current_uploads_;
    enqueued_to_start_uploads_.erase(request.get());
}

void UploadManagerImpl::DecrementUploadingCount(StateMachinePointer)
{
    EnqueueTick();

    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    --current_uploads_;
}

// -- END UploadStateMachineCallbackInterface ----------------------------------

}  // namespace detail
}  // namespace uploader
}  // namespace mf
