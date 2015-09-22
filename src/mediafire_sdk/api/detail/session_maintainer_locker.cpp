/**
 * @file session_maintainer_locker.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "session_maintainer_locker.hpp"

#include "boost/variant/get.hpp"

#include "mediafire_sdk/utils/variant_comparison.hpp"

#ifdef OUTPUT_DEBUG
#  include "boost/current_function.hpp"
#  define DEBUG_TOKEN_COUNT() \
        do { \
            std::cout << BOOST_CURRENT_FUNCTION << "\n"; \
            DebugOutputTokenCounts(); \
        } while ( false )
#else
#  define DEBUG_TOKEN_COUNT()
#endif

namespace mf {
namespace api {
namespace detail {

SessionMaintainerLocker::SessionMaintainerLocker(
        boost::asio::io_service * work_ios,
        boost::asio::io_service * callback_ios,
        TimedEvents::EventProcessor event_processor
    ) :
    callback_ios_(callback_ios),
    session_state_(session_state::Uninitialized()),
    connection_state_(connection_state::Uninitialized()),
    session_state_change_count_(0),
    delayed_requests_(TimedEvents::Create( work_ios, event_processor)),
    in_progress_session_token_requests_(0),
    time_out_requests_(WeakTimedEvents::Create( work_ios,
            CreateTimeoutProcessor())),
    failure_count_(0)
{
}

SessionMaintainerLocker::~SessionMaintainerLocker()
{
    StopTimeouts();

    // Cancel all requests
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);
    auto old_waiting_st_requests = std::move(waiting_st_requests_);
    auto old_waiting_non_st_requests = std::move(waiting_non_st_requests_);
    auto in_progress_requests = std::move(in_progress_requests_);
    lock.unlock();

    for (auto & request : old_waiting_st_requests)
        request->Cancel();
    for (auto & request : old_waiting_non_st_requests)
        request->Cancel();
    for (auto & request : in_progress_requests)
        request->Cancel();
}

SessionMaintainerLocker::WeakTimedEvents::EventProcessor
SessionMaintainerLocker::CreateTimeoutProcessor()
{
    // Safe to pass in this as we call Stop in DTOR.
    return [this](
        STRequestWeak weak_request,
        const std::error_code & ec
    )
    {
        if (!ec)
            HandleTimedOutRequest(weak_request);
    };
}

boost::optional<Credentials> SessionMaintainerLocker::GetCredenials()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    return credentials_;
}

void SessionMaintainerLocker::SetCredentials(
        const Credentials & credentials
    )
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);

    bool credentials_did_change = ( !credentials_ ||
        !mf::utils::AreVariantsEqual(credentials, *credentials_));
    bool state_is_running =
        boost::get<session_state::Running>(&session_state_) != nullptr;
    if ( credentials_did_change || ! state_is_running )
    {
        credentials_ = credentials;

        // Change state to initialized.
        ChangeSessionStateInternal(session_state::Initialized(), lock);
    }
}

void SessionMaintainerLocker::AddWaitingRequest( STRequest request )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

    time_out_requests_->Add(
        std::chrono::seconds(request->TimeoutSeconds()),
        STRequestWeak(request) );

    if (request->UsesSessionToken())
    {
        waiting_st_requests_.push_back( request );
        DEBUG_TOKEN_COUNT();
    }
    else
    {
        waiting_non_st_requests_.push_back( request );
    }
}

void SessionMaintainerLocker::RemoveInProgressRequest( STRequest request )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    in_progress_requests_.erase( request );
}

void SessionMaintainerLocker::MoveInProgressToWaiting( STRequest request )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    in_progress_requests_.erase( request );

    if (request->UsesSessionToken())
    {
        waiting_st_requests_.push_back( request );
        DEBUG_TOKEN_COUNT();
    }
    else
    {
        waiting_non_st_requests_.push_back( request );
    }
}

void SessionMaintainerLocker::DeleteCheckedOutToken( STRequest request )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    auto it = checked_out_tokens_.find( request );
    if ( it != checked_out_tokens_.end() )
    {
        checked_out_tokens_.erase( it );
    }
    DEBUG_TOKEN_COUNT();
}

void SessionMaintainerLocker::MoveInProgressToDelayed(
        STRequest request,
        int delay_sec
    )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    in_progress_requests_.erase( request );
    delayed_requests_->Add( std::chrono::seconds(delay_sec), request );
}

boost::optional< STRequest > SessionMaintainerLocker::NextWaitingNonSessionTokenRequest()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

    if ( ! waiting_non_st_requests_.empty() )
    {
        STRequest request = waiting_non_st_requests_.front();
        waiting_non_st_requests_.pop_front();

        return request;
    }

    return boost::none;
}

boost::optional<std::pair<STRequest, SessionTokenData>>
SessionMaintainerLocker::NextWaitingSessionTokenRequest()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

    if ( ! waiting_st_requests_.empty() && ! session_tokens_.empty() )
    {
        SessionTokenData st = session_tokens_.back();
        session_tokens_.pop_back();

        STRequest request = std::move(waiting_st_requests_.front());
        waiting_st_requests_.pop_front();

        DEBUG_TOKEN_COUNT();

        return std::make_pair(std::move(request), std::move(st));
    }
    else
    {
#ifdef OUTPUT_DEBUG
        std::cout << "Returning no waiting requests because:\n";
        DEBUG_TOKEN_COUNT();
#endif
        return boost::none;
    }
}

void SessionMaintainerLocker::AddInProgressRequest( STRequest request )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    in_progress_requests_.insert( request );
}

void SessionMaintainerLocker::AddInProgressRequest(
        STRequest request,
        SessionTokenData token
    )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    in_progress_requests_.insert( request );
    checked_out_tokens_.insert(
        std::make_pair(std::move(request), std::move(token) ) );
    DEBUG_TOKEN_COUNT();
}

bool SessionMaintainerLocker::StartRequestSessionToken()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

    auto NoSessionTokensOrRequests = [this]() -> bool
    {
        // Have at least one session token at all times
        return (in_progress_session_token_requests_ == 0
                && session_tokens_.size() + checked_out_tokens_.size() == 0);
    };

    auto MoreEnqueuedThanAvailable = [this]() -> bool
    {
        /* Are there more requests than session tokens and session token
           requests?  No check against checked out tokens, as we allow maximum
           concurrent requests. */
        return (waiting_st_requests_.size()
                > in_progress_session_token_requests_ + session_tokens_.size());
    };

    auto LessRequestsThanMax = [this]() -> bool
    {
        /* Are there less session token requests than maximum concurrent session
           token requests allowed? */
        return (in_progress_session_token_requests_
                < SessionMaintainer::max_in_progress_token_requests);
    };

    auto LessTotalTokensThanMax = [this]() -> bool
    {
        /* Are there less session token requests and session tokens than the
           maximum allowed session tokens? */
        return (in_progress_session_token_requests_ + session_tokens_.size()
                        + checked_out_tokens_.size()
                < SessionMaintainer::max_tokens);
    };

    if (NoSessionTokensOrRequests()
        || (MoreEnqueuedThanAvailable() && LessRequestsThanMax()
            && LessTotalTokensThanMax()))
    {
        ++in_progress_session_token_requests_;
        return true;
    }
    else
    {
        return false;
    }
}

void SessionMaintainerLocker::DecrementSessionTokenInProgressCount()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    assert(in_progress_session_token_requests_ > 0);
    --in_progress_session_token_requests_;
}

bool SessionMaintainerLocker::AddSessionToken(
        SessionTokenData token,
        Credentials old_credentials
    )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

    if ( credentials_ &&
        mf::utils::AreVariantsEqual( old_credentials, *credentials_ ) )
    {
        // Check to ensure we aren't inserting more than one of these
        assert(
            std::find_if(
                std::begin(session_tokens_),
                std::end(session_tokens_),
                [&token](const SessionTokenData & it)
                {
                    return it.session_token == token.session_token;
                }) == std::end(session_tokens_));
        session_tokens_.push_back( std::move(token) );
        DEBUG_TOKEN_COUNT();
        return true;
    }
    else
    {
        return false;
    }
}

void SessionMaintainerLocker::ReuseToken(
        STRequest request,
        ResponseBase * response
    )
{
    if (request->UsesSessionToken())
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

        auto it = checked_out_tokens_.find( request );
        if ( it != checked_out_tokens_.end() )
        {
            SessionTokenData st;
            std::swap( st, it->second );
            checked_out_tokens_.erase( it );

            // If a key was set to be renewed, we must modify the secret
            // key.
            if ( PropertyHasValue(
                    response->pt,
                    "response.new_key",
                    std::string("yes") ) )
            {
                uint64_t current_key = st.secret_key;
                uint64_t next_key = (current_key * 16807) % 2147483647;
                st.secret_key = static_cast<int>(next_key);
            }

            // Check to ensure we aren't inserting more than one of these
            assert(
                std::find_if(
                    std::begin(session_tokens_),
                    std::end(session_tokens_),
                    [&st](const SessionTokenData & it)
                    {
                        return it.session_token == st.session_token;
                    }) == std::end(session_tokens_));
            session_tokens_.push_back( std::move(st) );

            DEBUG_TOKEN_COUNT();
        }
    }
}

std::size_t SessionMaintainerLocker::TotalRequests()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    return waiting_st_requests_.size()
        + waiting_non_st_requests_.size()
        + in_progress_requests_.size()
        + delayed_requests_->Size();
}

std::pair<api::SessionState, uint32_t>
SessionMaintainerLocker::GetSessionState()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    return std::make_pair(session_state_, session_state_change_count_);
}

void SessionMaintainerLocker::SetSessionState(api::SessionState state)
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);
    ChangeSessionStateInternal(state, lock);
}

api::ConnectionState SessionMaintainerLocker::GetConnectionState()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    return connection_state_;
}

void SessionMaintainerLocker::SetConnectionState(
        const api::ConnectionState & state
    )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    ChangeConnectionStateInternal(state);
}

bool SessionMaintainerLocker::SetSessionStateSafe(
        api::SessionState state,
        uint32_t state_change_count
    )
{
    mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);
    if (session_state_change_count_ == state_change_count)
    {
        ChangeSessionStateInternal(state, lock);
        return true;
    }
    return false;
}

void SessionMaintainerLocker::SetSessionStateChangeCallback(
        SessionMaintainer::SessionStateChangeCallback callback
    )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    session_state_change_callback_ = callback;
}

void SessionMaintainerLocker::SetConnectionStateChangeCallback(
        SessionMaintainer::ConnectionStateChangeCallback callback
    )
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    connection_state_change_callback_ = callback;
}

void SessionMaintainerLocker::StopTimeouts()
{
    mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
    delayed_requests_->Stop();
    time_out_requests_->Stop();
}

class SessionMaintainerLocker::NewStateVisitor :
    public boost::static_visitor<>
{
public:
    // parent_->mutex_ must be locked
    NewStateVisitor(
            SessionMaintainerLocker* parent,
            const mf::utils::unique_lock<mf::utils::mutex>& lock
        ) :
        parent_(parent)
    {   // Empty constructor
    }
    void operator()(session_state::Uninitialized) const
    {   // Clear all tokens and credentials when uninitializing
        parent_->credentials_ = boost::none;
        parent_->session_tokens_.clear();
        parent_->checked_out_tokens_.clear();
    }

    void operator()(session_state::Initialized) const
    {   // Clear all tokens on init
        parent_->session_tokens_.clear();
        parent_->checked_out_tokens_.clear();
    }

    void operator()(session_state::CredentialsFailure) const
    {   // Clear all tokens on error
        parent_->session_tokens_.clear();
        parent_->checked_out_tokens_.clear();
    }

    void operator()(session_state::ProlongedError) const
    {   // This is just an adviosry state and should not affect behavior
    }

    void operator()(session_state::Running) const
    {   // Do nothing when finally running
    }
private:
    SessionMaintainerLocker* parent_;
};

void SessionMaintainerLocker::ChangeSessionStateInternal(
        api::SessionState state,
        const mf::utils::unique_lock<mf::utils::mutex>& lock
    )
{
    (void) lock;  // Suppress warnings
    assert( lock.owns_lock() );
    assert( lock.mutex() == &mutex_ );

    if ( ! mf::utils::AreVariantsEqual( session_state_, state ) )
    {
        session_state_ = state;
        boost::apply_visitor(NewStateVisitor(this, lock), state);
        ++session_state_change_count_;
        if (session_state_change_callback_)
            callback_ios_->post( boost::bind(
                    session_state_change_callback_,
                    session_state_) );
    }
}

void SessionMaintainerLocker::ChangeConnectionStateInternal(
        const api::ConnectionState & state
    )
{
    // No lock on this one since we should already be locked.
    if ( ! mf::utils::AreVariantsEqual( connection_state_, state) )
    {
        connection_state_ = state;
        if (connection_state_change_callback_)
            callback_ios_->post( boost::bind(
                    connection_state_change_callback_,
                    connection_state_) );
    }
}

void SessionMaintainerLocker::HandleTimedOutRequest(STRequestWeak weak_request)
{
    STRequest request = weak_request.lock();

    if (request)
    {
        if (request->UsesSessionToken())
        {
            mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);
            auto it = std::find(waiting_st_requests_.begin(),
                                waiting_st_requests_.end(), request);
            if (it != waiting_st_requests_.end())
            {
                waiting_st_requests_.erase(it);

                DEBUG_TOKEN_COUNT();

                std::ostringstream ss;
                ss << "No session token was available before the timeout was"
                      " reached.  Session state: " << session_state_
                   << "  Connection state: " << connection_state_
                   << "\n" << GetDebugOutputTokenCounts();

                // In case of Fail calling mutex
                lock.unlock();

                request->Fail(
                        make_error_code(
                                api::api_code::SessionTokenUnavailableTimeout),
                        ss.str());
            }
        }
        else
        {
            mf::utils::unique_lock<mf::utils::mutex> lock(mutex_);
            std::deque<STRequest>::iterator it
                    = std::find(waiting_non_st_requests_.begin(),
                                waiting_non_st_requests_.end(), request);
            if (it != waiting_non_st_requests_.end())
            {
                waiting_non_st_requests_.erase(it);

                // In case of Fail calling mutex
                lock.unlock();

                request->Fail(
                        make_error_code(
                                api::api_code::ConnectionUnavailableTimeout),
                        "connection unavailable before the timeout was "
                        "reached.");
            }
        }
    }
}

void SessionMaintainerLocker::IncrementFailureCount()
{
    failure_count_.fetch_add(1, boost::memory_order_release);
}

uint32_t SessionMaintainerLocker::GetFailureCount()
{
    return failure_count_.load(boost::memory_order_consume);
}

void SessionMaintainerLocker::ResetFailureCount()
{
    failure_count_.store(0, boost::memory_order_release);
}


// Only call when mutex_ is locked!
std::string SessionMaintainerLocker::GetDebugOutputTokenCounts()
{
    std::ostringstream ss;

    ss << "Pending token requests:    " << in_progress_session_token_requests_
       << "\n"

       << "Stored tokens:             " << session_tokens_.size() << "\n"

       << "Checked-out tokens:        " << checked_out_tokens_.size() << "\n"

       << "Requests waiting on token: " << waiting_st_requests_.size() << "\n"

       << "Max tokens:                " << SessionMaintainer::max_tokens
       << "\n";

    return ss.str();
}

// Only call when mutex_ is locked!
void SessionMaintainerLocker::DebugOutputTokenCounts()
{
    std::cout << GetDebugOutputTokenCounts();
}

}  // namespace detail
}  // namespace api
}  // namespace mf
