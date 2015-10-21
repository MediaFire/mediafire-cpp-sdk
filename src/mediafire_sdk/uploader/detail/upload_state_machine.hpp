/**
 * @file upload_state_machine.hpp
 * @author Herbert Jones
 * @brief State machine for uploading a file.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>
#include <random>
#include <vector>

#include "upload_events.hpp"

#include "mediafire_sdk/http/bandwidth_analyser_interface.hpp"

#include "boost/mpl/vector/vector30.hpp"
#include "boost/msm/back/state_machine.hpp"
#include "boost/msm/front/state_machine_def.hpp"
#include "boost/msm/front/euml/operator.hpp"
#include "boost/msm/front/functor_row.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/filesystem/path.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/uploader/upload_status.hpp"

#include "mediafire_sdk/uploader/detail/hasher_events.hpp"
#include "mediafire_sdk/uploader/detail/hasher_transitions.hpp"
#include "mediafire_sdk/uploader/detail/stepping.hpp"
#include "mediafire_sdk/uploader/detail/transitions.hpp"
#include "mediafire_sdk/uploader/detail/types.hpp"
#include "mediafire_sdk/uploader/detail/upload_target.hpp"

#include "mediafire_sdk/utils/string.hpp"

namespace mf {
namespace uploader {
namespace detail {

// Leave inside namespace to not pollute outer namespace.
namespace asio = ::boost::asio;
namespace mpl  = ::boost::mpl;
namespace msm  = ::boost::msm;
namespace hl   = ::mf::http;
namespace ut   = ::mf::uploader::detail::upload_transition;
namespace ht   = mf::uploader::detail::hash_transition;
namespace he   = mf::uploader::detail::hash_event;
namespace us   = ::mf::uploader::upload_state;


using msm::front::Row;
using msm::front::none;
using msm::front::euml::Not_;

// Forward declaration
class UploadStateMachine_;
class UploadStateMachineCallbackInterface;

// Pick a back-end
using UploadStateMachine = boost::msm::back::state_machine<UploadStateMachine_>;
using StateMachinePointer = std::shared_ptr<UploadStateMachine>;
using StateMachineWeakPointer = std::weak_ptr<UploadStateMachine>;

typedef std::function<void(::mf::uploader::UploadStatus)> StatusCallback;

struct UploadConfig
{
    UploadHandle upload_handle;
    UploadStateMachineCallbackInterface * callback_interface;
    ::mf::api::SessionMaintainer * session_maintainer;
    boost::filesystem::path filepath;
    OnDuplicateAction on_duplicate_action;
    StatusCallback status_callback;
    boost::optional<std::string> cloud_file_name;
    boost::optional<UploadTarget> target_folder;
};

struct ChunkData
{
    std::string hash;
    std::vector<std::pair<uint64_t,uint64_t>> chunk_ranges;
    std::vector<std::string> chunk_hashes;
};

/**
 * @interface UploadStateMachineCallbackInterface
 */
class UploadStateMachineCallbackInterface
{
public:
    UploadStateMachineCallbackInterface() {}
    virtual ~UploadStateMachineCallbackInterface() {}

    virtual void HandleAddToHash(StateMachinePointer) = 0;
    virtual void HandleRemoveToHash(StateMachinePointer) = 0;

    virtual void HandleAddToUpload(StateMachinePointer) = 0;
    virtual void HandleRemoveToUpload(StateMachinePointer) = 0;

    virtual void HandleComplete(StateMachinePointer) = 0;

    virtual void IncrementHashingCount(StateMachinePointer) = 0;
    virtual void DecrementHashingCount(StateMachinePointer) = 0;

    virtual void IncrementUploadingCount(StateMachinePointer) = 0;
    virtual void DecrementUploadingCount(StateMachinePointer) = 0;
};

// front-end: define the FSM structure
class UploadStateMachine_ :
    public std::enable_shared_from_this<UploadStateMachine_>,
    public msm::front::state_machine_def<UploadStateMachine_>
{
public:
#define CALLBACK_INTERFACE(method)                                             \
    if (callback_interface_)                                                   \
    {                                                                          \
        callback_interface_->method(AsFrontShared());                          \
    }
#define CALLBACK_INTERFACE_FSM(method)                                         \
    if (fsm.callback_interface_)                                               \
    {                                                                          \
        fsm.callback_interface_->method(fsm.AsFrontShared());                  \
    }


    typedef std::shared_ptr<UploadStateMachine_> Pointer;

    UploadStateMachine & AsFront()
    {
        return static_cast<msm::back::state_machine<UploadStateMachine_>&>(
            *this);
    }

    StateMachinePointer AsFrontShared()
    {
        std::shared_ptr<UploadStateMachine_> self = shared_from_this();
        return std::static_pointer_cast<UploadStateMachine>(self);
    }

    StateMachineWeakPointer AsFrontWeak()
    {
        auto self = shared_from_this();
        return StateMachineWeakPointer(
            std::static_pointer_cast<UploadStateMachine>(self));
    }

    // Warning: Not possible to add more than 5 arguments here:
    UploadStateMachine_(
            const UploadConfig & config
        ) :
        session_maintainer_(config.session_maintainer),
        http_config_(session_maintainer_->HttpConfig()),
        work_io_service_(http_config_->GetWorkIoService()),
        callback_io_service_(http_config_->GetDefaultCallbackIoService()),
        event_strand_(*http_config_->GetWorkIoService()),
        timer_(*work_io_service_),
        callback_interface_(config.callback_interface),
        upload_handle_(config.upload_handle),
        filepath_(config.filepath),
        on_duplicate_action_(config.on_duplicate_action),
        status_callback_(config.status_callback),
        cloud_file_name_(config.cloud_file_name),
        target_folder_(config.target_folder),
        filesize_(0),
        mtime_(0),
        count_state_(CountState::None)
    {
        assert(work_io_service_);
        assert(callback_io_service_);
        assert(callback_interface_);
        assert(!filepath_.empty());
    }

    template<typename Event>
    void ProcessEvent(const Event & event)
    {
        auto self = AsFrontShared();
        event_strand_.dispatch(
            [event, self]
            {
                self->process_event(event);
            });
    }

    template<typename Event>
    void PostEvent(const Event & event)
    {
        auto self = AsFrontShared();
        event_strand_.post(
            [event, self]
            {
                self->process_event(event);
            });
    }

    void SetCountState(CountState cs)
    {
        // Exit state range actions
        switch(count_state_)
        {
            case CountState::None:
                break;
            case CountState::Hashing:
                CALLBACK_INTERFACE(DecrementHashingCount);
                break;
            case CountState::Uploading:
                CALLBACK_INTERFACE(DecrementUploadingCount);
                break;
            default:
                assert(!"Unknown count state");
        }

        // Enter state range actions
        switch(cs)
        {
            case CountState::None:
                break;
            case CountState::Hashing:
                CALLBACK_INTERFACE(IncrementHashingCount);
                break;
            case CountState::Uploading:
                CALLBACK_INTERFACE(IncrementUploadingCount);
                break;
            default:
                assert(!"Unknown count state");
        }

        count_state_ = cs;
    }

    // -- States ---------------------------------------------------------------
    struct HashStart : public msm::front::state<> {};
    struct UploadFile : public msm::front::state<> {};
    struct InstantUpload : public msm::front::state<> {};

    struct Initial : public msm::front::state<>
    {
        template <typename Event, typename FSM>
        void on_exit(Event const &, FSM& fsm)
        {
            // Setup actions can go in here.

            boost::system::error_code bec;
            fsm.filesize_ = boost::filesystem::file_size(fsm.filepath_, bec);

            if (bec)
            {
                std::error_code ec(bec.value(), std::system_category());
                fsm.ProcessEvent(event::Error{
                    ec,
                    "Filsize unavailable."
                    });
                return;
            }

            fsm.mtime_ = boost::filesystem::last_write_time(fsm.filepath_, bec);

            if (bec)
            {
                std::error_code ec(bec.value(), std::system_category());
                fsm.ProcessEvent(event::Error{
                    ec,
                    "Mtime unavailable."
                    });
                return;
            }

            if (fsm.filesize_ == 0)
            {
                fsm.ProcessEvent(event::Error{
                    make_error_code(mf::uploader::errc::ZeroByteFile),
                    "API does not support empty files"
                    });
                return;
            }
        }
    };

    struct WaitForHashSignal : public msm::front::state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM& fsm)
        {
            CALLBACK_INTERFACE_FSM(HandleAddToHash);
            fsm.SendStatus(us::EnqueuedForHashing{});
        }

        template <typename Event, typename FSM>
        void on_exit(Event const &, FSM& fsm)
        {
            CALLBACK_INTERFACE_FSM(HandleRemoveToHash);
        }
    };

    struct SetupHasher : public msm::front::state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM& fsm)
        {
            fsm.SetCountState(CountState::Hashing);

            auto state = std::make_shared<he::HasherStateData_>();
            state->io_service = fsm.work_io_service_;
            state->filepath = fsm.filepath_;

            state->filesize = fsm.filesize_;
            state->mtime = fsm.mtime_;

            state->chunk_ranges = mf::uploader::detail::ChunkRanges(
                state->filesize);

            fsm.ProcessEvent(he::StartHash{std::move(state)});
        }
    };

    struct Hashing : public msm::front::state<>
    {
        template <typename FSM>
        void on_entry(he::StartHash const &, FSM& fsm)
        {
            fsm.SendStatus(us::Hashing{});
        }
        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM&)
        {
        }
    };

    struct WaitForUploadSignal : public msm::front::state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM& fsm)
        {
            CALLBACK_INTERFACE_FSM(HandleAddToUpload);
            fsm.SendStatus(us::EnqueuedForUpload{});
        }

        template <typename Event, typename FSM>
        void on_exit(Event const &, FSM& fsm)
        {
            CALLBACK_INTERFACE_FSM(HandleRemoveToUpload);
        }
    };

    struct InUploadCheck : public msm::front::state<>
    {
        template <typename FSM>
        void on_entry(event::StartUpload const & evt, FSM& fsm)
        {
            assert( ! evt.upload_action_token.empty() );

            fsm.action_token_ = evt.upload_action_token;

            fsm.SetCountState(CountState::Uploading);

            fsm.SendStatus(us::Uploading{});
        }

        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM&)
        {
            static_assert(std::is_same<FSM, bool>::value, /* Impossible
                 condition forces static assert if this on_entry is created. */
                "Unhandled on_entry_type");
        }
    };

    struct UploadChunk : public msm::front::state<>
    {
        template <typename FSM>
        void on_entry(event::ChunkSuccess const & evt, FSM& fsm)
        {
            // On chunk success, set the chunk state.
            assert( fsm.GetChunkState(evt.chunk_id) == ChunkState::Uploading);
            assert( ! evt.upload_key.empty() );

            if (fsm.upload_key_.empty())
                fsm.upload_key_ = evt.upload_key;
            fsm.SetChunkState(evt.chunk_id, ChunkState::Uploaded);
        }

        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM&)
        {
        }

        template <typename Event, typename FSM>
        void on_exit(Event const &, FSM& fsm)
        {
            // On exit, clean up ongoing uploads.
            if (fsm.upload_request_)
            {
                fsm.upload_request_->Cancel();
                fsm.upload_request_.reset();
            }
        }
    };

    struct PollUpload : public msm::front::state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const &, FSM& fsm)
        {
            fsm.SendStatus(us::Polling{});
        }
    };

    // -- Final States --
    struct CompleteWithSuccess : public msm::front::terminate_state<>
    {
        template <typename FSM>
        void on_entry(event::AlreadyUploaded const & evt, FSM & fsm)
        {
            auto fsmp = fsm.AsFrontShared();  // Ensures fsm isn't deleted by
                                              // callback

            assert(!evt.quickkey.empty());

            fsmp->SetCountState(CountState::None);

            fsmp->SendStatus(us::Complete{
                    evt.quickkey, evt.filename, fsmp->hash_, boost::none});

            CALLBACK_INTERFACE_FSM(HandleComplete);
        }

        template <typename Event, typename FSM>
        void on_entry(Event const & evt, FSM & fsm)
        {
            auto fsmp = fsm.AsFrontShared();  // Ensures fsm isn't deleted by
                                              // callback

            assert(!evt.quickkey.empty());

            fsmp->SetCountState(CountState::None);

            fsmp->SendStatus(us::Complete{evt.quickkey,
                                          evt.filename,
                                          fsmp->hash_,
                                          evt.new_device_revision});
            CALLBACK_INTERFACE_FSM(HandleComplete);

        }
    };

    struct CompleteWithError : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const & evt, FSM & fsm)
        {
            auto fsmp = fsm.AsFrontShared();  // Ensures fsm isn't deleted by
                                              // callback

            fsmp->SetCountState(CountState::None);

            // Report hash if we have it.
            boost::optional<std::string> hash;
            if (!fsmp->hash_.empty())
                hash = fsmp->hash_;

            fsmp->SendStatus(us::Error{
                    evt.error_code, evt.description, std::move(hash)});

            CALLBACK_INTERFACE_FSM(HandleComplete);
        }
    };

    // -- END States -----------------------------------------------------------

    // -- Guards ---------------------------------------------------------------
    struct HasHash
    {
        template <class Event,class FSM,class SourceState,class TargetState>
        bool operator()(Event const&, FSM& fsm, SourceState&, TargetState&)
        {
            return ! fsm.hash_.empty();
        }
    };
    // -- END Guards -----------------------------------------------------------

    // the initial state of the StateMachine SM. Must be defined
    typedef Initial initial_state;

    // Transition table for StateMachine
    struct transition_table : mpl::vector25<
        //    Start               , Event                       , Next                , Action              , Guard
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < Initial             , event::Start                , WaitForHashSignal   , none                , Not_<HasHash>      >,
        Row < Initial             , event::Start                , WaitForUploadSignal , none                , HasHash            >,
        Row < Initial             , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < WaitForHashSignal   , event::StartHash            , SetupHasher         , none                , none               >,
        Row < WaitForHashSignal   , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < SetupHasher         , he::StartHash               , Hashing             , ht::OpenFile        , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < Hashing             , he::ReadNext                , Hashing             , ht::ReadFile        , none               >,
        Row < Hashing             , he::HashSuccess             , WaitForUploadSignal , ut::SaveHash        , none               >,
        Row < Hashing             , he::Error                   , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < WaitForUploadSignal , event::StartUpload          , InUploadCheck       , ut::Check           , none               >,
        Row < WaitForUploadSignal , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < InUploadCheck       , event::NeedsSingleUpload    , UploadFile          , ut::DoSimpleUpload  , none               >,
        Row < InUploadCheck       , event::NeedsChunkUpload     , UploadChunk         , ut::DoChunkUpload   , none               >,

        Row < InUploadCheck       , event::InstantUpload        , InstantUpload       , ut::DoInstantUpload , none               >,
        Row < InUploadCheck       , event::AlreadyUploaded      , CompleteWithSuccess , none                , none               >,
        Row < InUploadCheck       , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < InstantUpload       , event::InstantSuccess       , CompleteWithSuccess , none                , none               >,
        Row < InstantUpload       , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < UploadChunk         , event::ChunkSuccess         , UploadChunk         , ut::DoChunkUpload   , none               >,
        Row < UploadChunk         , event::ChunkUploadComplete  , PollUpload          , ut::PollUpload      , none               >,
        Row < UploadChunk         , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < UploadFile          , event::SimpleUploadComplete , PollUpload          , ut::PollUpload      , none               >,
        Row < UploadFile          , event::Error                , CompleteWithError   , none                , none               >,
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
        Row < PollUpload          , event::PollComplete         , CompleteWithSuccess , none                , none               >,
        Row < PollUpload          , event::Error                , CompleteWithError   , none                , none               >
        // ---------------------- , --------------------------- , ------------------- , ------------------  , ------------------
    > {};

#ifdef NO_STATE_ASSERT
    // Replaces the default no-transition response. Use this if you don't
    // want the machine to assert on an unexpected event.
    template <typename FSM, typename Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cerr << "no transition from state " << state << " on event "
            << typeid(e).name() << std::endl;
        assert(!"improper transition in uploader state machine");
    }
#endif

    /** Remove all callbacks to the upload manager. */
    void Disconnect()
    {
        callback_interface_ = nullptr;
    }

    UploadHandle Handle() const {return upload_handle_;}

    boost::filesystem::path Path() const
    {
        return filepath_;
    }

    std::string filename() const
    {
        if (cloud_file_name_)
            return *cloud_file_name_;
        else
            return mf::utils::path_to_utf8(filepath_.filename());
    }

    UploadStateMachineCallbackInterface * GetCallbackInterface()
    {
        return callback_interface_;
    }

    void SetCompleteHashData(
            std::string hash,
            he::HasherStateData hash_data
        )
    {
        hash_ = hash;
        chunk_ranges_ = hash_data->chunk_ranges;
        chunk_hashes_ = hash_data->chunk_hashes;
    }

    ::mf::api::SessionMaintainer * GetSessionMaintainer() const
    {
        assert(session_maintainer_);
        return session_maintainer_;
    }

    std::string hash() const
    {
        assert( ! hash_.empty() );
        return hash_;
    }

    const std::vector<std::pair<uint64_t,uint64_t>> & chunkRanges() const
    {
        assert( ! chunk_ranges_.empty() );
        return chunk_ranges_;
    }

    const std::vector<std::string> & chunkHashes() const
    {
        assert( ! chunk_hashes_.empty() );
        return chunk_hashes_;
    }

    UploadTarget targetFolder() const
    {
        if (target_folder_)
            return *target_folder_;
        else
            return ParentFolderKey{""};
    }

    uint64_t filesize() const {return filesize_;}

    std::time_t mtime() const {return mtime_;}

    OnDuplicateAction onDuplicateAction() const {return on_duplicate_action_;}

    void SetBitmap(const std::vector<uint16_t> & bitmap)
    {
        chunk_states_.clear();
        for (const auto word : bitmap)
        {
            // There is no reason the number of bits we are given should be more
            // than the number of chunks, ignoring the excess bits in the last
            // word.
            assert(chunk_states_.size() < chunk_ranges_.size());

            uint16_t mask = 1;
            for (int i = 0; i < 16; ++i)
            {
                // Stop if we are in the last word and have all the expected
                // bits.
                if (chunk_states_.size() == chunk_ranges_.size())
                    break;

                if (mask & word)
                    chunk_states_.push_back(ChunkState::Uploaded);
                else
                    chunk_states_.push_back(ChunkState::NeedsUpload);

                mask <<= 1;
            }
        }

        for (unsigned int i = chunk_states_.size(); i < chunk_ranges_.size(); ++i )
            chunk_states_.push_back(ChunkState::NeedsUpload);

        // Under all circumstances we should have the same number of bits as
        // chunk ranges.
        assert(chunk_states_.size() == chunk_ranges_.size());
    }

    // Only updates existing with completed chunks
    void UpdateBitmap(const std::vector<uint16_t> & bitmap)
    {
        assert( ! chunk_states_.empty() );

        uint32_t pos = 0;
        for (const auto word : bitmap)
        {
            uint16_t mask = 1;
            for (int i = 0; i < 16; ++i)
            {
                assert( pos < chunk_states_.size() );
                // Stop if we are in the last word and have all the expected
                // bits.
                if (pos == chunk_ranges_.size())
                    break;

                if (mask & word && chunk_states_.at(pos) == ChunkState::NeedsUpload)
                    chunk_states_.at(pos) = ChunkState::Uploaded;

                mask <<= 1;
                ++pos;
            }
        }
    }

    void SetChunkState(uint32_t chunk_id, ChunkState chunk_state)
    {
        assert(chunk_id < chunk_states_.size());
        try{
            chunk_states_.at(chunk_id) = chunk_state;
        }
        catch (std::out_of_range &)
        {
            ProcessEvent(event::Error{
                make_error_code(mf::uploader::errc::LogicError),
                "Requested chunk to set not in range."
                });
        }
    }

    ChunkState GetChunkState(uint32_t chunk_id)
    {
        assert(chunk_id < chunk_states_.size());
        try{
            return chunk_states_.at(chunk_id);
        }
        catch (std::out_of_range &)
        {
            ProcessEvent(event::Error{
                make_error_code(mf::uploader::errc::LogicError),
                "Requested chunk to read not in range."
                });
            return ChunkState::Uploaded;
        }
    }

    boost::optional<uint32_t> NextChunkToUpload()
    {
        // Return a chunk id to upload.  Pick next chunk randomly so multiple
        // uploads of the same file are less likely to upload the same chunk at
        // the same time.
        std::vector<uint32_t> incomplete_chunks;
        uint32_t chunk_id = 0;
        for (const auto state : chunk_states_)
        {
            if (state == ChunkState::NeedsUpload)
                incomplete_chunks.push_back(chunk_id);

            ++chunk_id;
        }

        if (incomplete_chunks.empty())
            return boost::none;

        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, incomplete_chunks.size()-1);
        auto ret_id = incomplete_chunks.at(dist(rd));

        return ret_id;
    }

    asio::io_service * IoService() const {return work_io_service_;}

    void SetUploadRequest(mf::http::HttpRequest::Pointer v) {upload_request_=v;}

    std::string ActionToken() const {return action_token_;}

    std::string UploadKey() const {return upload_key_;}

    asio::steady_timer * Timer() {return &timer_;}

    void SendStatus(const UploadState & state)
    {
        callback_io_service_->dispatch( boost::bind( status_callback_,
                UploadStatus{ upload_handle_, filepath_, state }));
    }

    ChunkData GetChunkData() const
    {
        return ChunkData { hash_, chunk_ranges_, chunk_hashes_ };
    }

protected:
    ::mf::api::SessionMaintainer * session_maintainer_;

    mf::http::HttpConfig::ConstPointer http_config_;

    // IOService for work.
    asio::io_service * work_io_service_;
    asio::io_service * callback_io_service_;

    // Strand for events.
    asio::io_service::strand event_strand_;

    // Timer for various timeouts.
    asio::steady_timer timer_;

    UploadStateMachineCallbackInterface * callback_interface_;

    const UploadHandle upload_handle_;

    // Path to local file
    boost::filesystem::path filepath_;

    std::string action_token_;

    OnDuplicateAction on_duplicate_action_;

    StatusCallback status_callback_;

    boost::optional<std::string> cloud_file_name_;

    boost::optional<UploadTarget> target_folder_;

    uint64_t filesize_;
    std::time_t mtime_;

    std::string hash_;
    std::vector<std::pair<uint64_t,uint64_t>> chunk_ranges_;
    std::vector<std::string> chunk_hashes_;

    CountState count_state_;

    std::vector<ChunkState> chunk_states_;

    mf::http::HttpRequest::Pointer upload_request_;

    std::string upload_key_;
};

}  // namespace detail
}  // namespace uploader
}  // namespace mf
