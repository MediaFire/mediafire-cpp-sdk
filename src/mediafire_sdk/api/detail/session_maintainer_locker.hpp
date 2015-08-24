/**
 * @file session_maintainer_locker.hpp
 * @author Herbert Jones
 * @brief Threadsafe impl for session maintainer
 * @copyright Copyright 2014 Mediafire
 */
#pragma once
#include <boost/atomic.hpp>

#include "mediafire_sdk/api/connection_state.hpp"
#include "mediafire_sdk/api/credentials.hpp"
#include "mediafire_sdk/api/detail/session_maintainer_request.hpp"
#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/session_state.hpp"
#include "mediafire_sdk/utils/timed_events.hpp"

namespace mf {
namespace api {
namespace detail {

class SessionMaintainerLocker
{
public:
    using TimedEvents = mf::utils::TimedEvents<STRequest>;
    using WeakTimedEvents = mf::utils::TimedEvents<STRequestWeak>;

    SessionMaintainerLocker(
            boost::asio::io_service * work_ios,
            boost::asio::io_service * callback_ios,
            TimedEvents::EventProcessor event_processor
        );

    ~SessionMaintainerLocker();

    WeakTimedEvents::EventProcessor CreateTimeoutProcessor();

    boost::optional<Credentials> GetCredenials();

    void SetCredentials(
            const Credentials & credentials
        );

    void AddWaitingRequest( STRequest request );

    void RemoveInProgressRequest( STRequest request );

    void MoveInProgressToWaiting( STRequest request );

    void DeleteCheckedOutToken( STRequest request );

    void MoveInProgressToDelayed( STRequest request, int delay_sec );

    boost::optional< STRequest > NextWaitingNonSessionTokenRequest();

    boost::optional< std::pair<STRequest, SessionTokenData> >
        NextWaitingSessionTokenRequest();

    void AddInProgressRequest( STRequest request );

    void AddInProgressRequest( STRequest request, SessionTokenData token );

    bool StartRequestSessionToken();

    void DecrementSessionTokenInProgressCount();

    bool AddSessionToken( SessionTokenData token, Credentials old_credentials );

    void ReuseToken(
            STRequest request,
            ResponseBase * response
        );

    std::size_t TotalRequests();

    std::pair<api::SessionState, uint32_t> GetSessionState();

    void SetSessionState(api::SessionState state);

    api::ConnectionState GetConnectionState();

    void SetConnectionState(
            const api::ConnectionState & state
        );

    bool SetSessionStateSafe(
            api::SessionState state,
            uint32_t state_change_count
        );

    void SetSessionStateChangeCallback(
            SessionMaintainer::SessionStateChangeCallback callback
        );

    void SetConnectionStateChangeCallback(
            SessionMaintainer::ConnectionStateChangeCallback callback
        );

    void StopTimeouts();

    void IncrementFailureCount();
    uint32_t GetFailureCount();
    void ResetFailureCount();

private:
    boost::asio::io_service * callback_ios_;

    mf::utils::mutex mutex_;

    // Set only via ChangeSessionStateInternal
    api::SessionState session_state_;

    // Set only via ChangeConnectionStateInternal
    api::ConnectionState connection_state_;

    uint32_t session_state_change_count_;
    SessionMaintainer::SessionStateChangeCallback
        session_state_change_callback_;

    SessionMaintainer::ConnectionStateChangeCallback
        connection_state_change_callback_;

    boost::optional<Credentials> credentials_;

    std::deque< STRequest > waiting_st_requests_;
    std::deque< STRequest > waiting_non_st_requests_;
    std::set< STRequest > in_progress_requests_;

    std::map< STRequest, SessionTokenData > checked_out_tokens_;

    // This contains the list of requests that were delayed for retrying later.
    TimedEvents::Pointer delayed_requests_;

    std::vector< SessionTokenData > session_tokens_;

    std::size_t in_progress_session_token_requests_;

    WeakTimedEvents::Pointer time_out_requests_;

    boost::atomic<uint32_t> failure_count_;

    void ChangeSessionStateInternal(
            api::SessionState state,
            const mf::utils::unique_lock<mf::utils::mutex>& lock
        );

    void ChangeConnectionStateInternal(
            const api::ConnectionState & state
        );

    void HandleTimedOutRequest(
            STRequestWeak weak_request
        );

    std::string GetDebugOutputTokenCounts();
    void DebugOutputTokenCounts();

    class NewStateVisitor;
};

}  // namespace detail
}  // namespace api
}  // namespace mf
