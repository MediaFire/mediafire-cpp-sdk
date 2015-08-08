/**
 * @file session_maintainer.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_maintainer.hpp"

#include <algorithm>
#include <chrono>
#include <deque>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "boost/current_function.hpp"
#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/api/detail/session_maintainer_locker.hpp"
#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/utils/variant_comparison.hpp"

#if !defined(NDEBUG)
// Counter to keep track of number of HttpRequests
boost::atomic<int> mf::api::detail::session_maintainer_request_count(0);
#endif

namespace hl = mf::http;

using mf::api::detail::STRequest;
using mf::api::detail::SessionMaintainerRequest;

namespace
{
namespace session_state = mf::api::session_state;
using mf::api::SessionState;

namespace connection_state = mf::api::connection_state;
using mf::api::ConnectionState;

const uint32_t skProlongedFailureThreshold = 10;

bool IsUninitialized(const SessionState & state)
{
    return (boost::get<session_state::Uninitialized>(&state));
}
bool IsInitialized(const SessionState & state)
{
    return (boost::get<session_state::Initialized>(&state));
}
bool IsCredentialsError(const SessionState & state)
{
    return (boost::get<session_state::CredentialsFailure>(&state));
}
bool IsProlongedError(const SessionState & state)
{
    return (boost::get<session_state::ProlongedError>(&state));
}
bool IsRunning(const SessionState & state)
{
    return (boost::get<session_state::Running>(&state));
}

bool IsConnected(const ConnectionState & state)
{
    return (boost::get<connection_state::Connected>(&state));
}
bool IsUnconnected(const ConnectionState & state)
{
    return (boost::get<connection_state::Unconnected>(&state));
}
}  // namespace

namespace mf
{
namespace api
{

SessionMaintainer::SessionMaintainer(
        mf::http::HttpConfig::ConstPointer http_config)
        : SessionMaintainer(http_config, "www.mediafire.com")
{
}

SessionMaintainer::SessionMaintainer(
        mf::http::HttpConfig::ConstPointer http_config,
        std::string hostname)
        : locker_(new detail::SessionMaintainerLocker(
                  http_config->GetWorkIoService(),
                  http_config->GetDefaultCallbackIoService(),
                  [this](STRequest request, const std::error_code & ec)
                  {
                      if (!ec)
                          AddWaitingRequest(request);
                  })),
          http_config_(http_config),
          session_token_failure_timer_(*http_config_->GetWorkIoService()),
          connection_state_recheck_timer_(*http_config_->GetWorkIoService()),
          requester_(http_config_, hostname),
          timeout_seconds_(60),
          maintainer_exists_(std::make_shared<Exists>(Exists::Yes))
{
    assert(http_config);

    auto maintainer_exists = maintainer_exists_;

    // Setup request callbacks
    on_stop_request_callback_ = [this, maintainer_exists](
            detail::STRequest request, ResponseBase * response)
    {
        if (*maintainer_exists == Exists::Yes)
            HandleCompletion(request, response);
        else
            request->Cancel();
    };

    on_retry_request_callback_ = [this, maintainer_exists](
            detail::STRequest request, ResponseBase * response)
    {
        if (*maintainer_exists == Exists::Yes)
            HandleRetryRequest(request, response);
        else
            request->Cancel();
    };

    info_update_callback_ = [this, maintainer_exists](detail::STRequest request,
                                                      ResponseBase * response)
    {
        if (*maintainer_exists == Exists::Yes)
            HandleCompletionNotification(request, response);
    };
}

SessionMaintainer::~SessionMaintainer()
{
    *maintainer_exists_ = Exists::No;
    session_token_failure_timer_.cancel();
}

void SessionMaintainer::SetLoginCredentials(const Credentials & credentials)
{
    locker_->SetCredentials(credentials);

    // Credentials updated.  There may be requests ready to send.
    AttemptRequests();
}

void SessionMaintainer::HandleCompletion(STRequest request,
                                         ResponseBase * response)
{
    locker_->RemoveInProgressRequest(request);

    // Reuse token later if possible
    if (IsInvalidSessionTokenError(response->error_code))
        locker_->DeleteCheckedOutToken(request);
    else
        locker_->ReuseToken(request, response);

    // Request done, start another if possible
    AttemptRequests();
}

void SessionMaintainer::HandleRetryRequest(STRequest request,
                                           ResponseBase * response)
{
    // Put the request back on the queue. Give it a new token when available.
    {
        if (IsInvalidSessionTokenError(response->error_code))
        {  // Token invalid- delete it
            locker_->MoveInProgressToWaiting(request);
            locker_->DeleteCheckedOutToken(request);
        }
        else
        {  // Reuse token later
            locker_->MoveInProgressToDelayed(request, 1);
            locker_->ReuseToken(request, response);
        }
    }

    // Request done, start another if possible
    AttemptRequests();
}

void SessionMaintainer::HandleCompletionNotification(STRequest /* request */,
                                                     ResponseBase * response)
{
    UpdateConnectionStateFromErrorCode(response->error_code);
}

void SessionMaintainer::UpdateConnectionStateFromErrorCode(
        const std::error_code & ec)
{
    using hl::http_error;

    // Check for errors that we count as connection errors
    if (ec.category() == hl::http_category()
        && (ec == http_error::UnableToConnect
            || ec == http_error::UnableToConnectToProxy
            || ec == http_error::UnableToResolve
            || ec == http_error::SslHandshakeFailure
            || ec == http_error::WriteFailure || ec == http_error::ReadFailure
            || ec == http_error::ProxyProtocolFailure
            || ec == http_error::IoTimeout))
    {
        connection_state::Unconnected new_state = {ec};

        SetConnectionState(new_state);

        // State is disconnected, so schedule a state check later
        connection_state_recheck_timer_.expires_from_now(
                std::chrono::milliseconds(connection_state_recheck_timeout_ms));
        connection_state_recheck_timer_.async_wait(boost::bind(
                &SessionMaintainer::HandleConnectionStateRecheckTimeout, this,
                boost::asio::placeholders::error));
    }
    else
    {
        SetConnectionState(connection_state::Connected());
    }
}

#ifdef OUTPUT_DEBUG
#define ATTEMPT_REQUEST_DEBUG(x)                                               \
    std::cout << "SessionMaintainer::AttemptRequests: " << x;
#else
#define ATTEMPT_REQUEST_DEBUG(x)
#endif

void SessionMaintainer::AttemptRequests()
{
    ATTEMPT_REQUEST_DEBUG("-- BEGIN --\n")

    // If disconnected, just attempt to reconnect
    if (IsUnconnected(GetConnectionState()))
    {
        AttemptConnection();
        return;
    }

    // Handle requests that do not use session tokens.
    boost::optional<STRequest> opt_request;
    while ((opt_request = locker_->NextWaitingNonSessionTokenRequest()))
    {
        STRequest & request = *opt_request;

        request->Init(&requester_);
        locker_->AddInProgressRequest(std::move(request));
        ATTEMPT_REQUEST_DEBUG("Started non-session token request.\n")
    }

    // Handle requests that have session tokens.
    boost::optional<std::pair<STRequest, SessionTokenData>> request_pair;
    ATTEMPT_REQUEST_DEBUG("Session state: "
                          << GetSessionState() << " is running: "
                          << IsRunning(GetSessionState()) << "\n")

    while (IsRunning(GetSessionState())
           && (request_pair = locker_->NextWaitingSessionTokenRequest()))
    {
        ATTEMPT_REQUEST_DEBUG("Found waiting session token request.\n")
        SessionTokenData & st = request_pair->second;
        STRequest & request = request_pair->first;

        request->SetSessionToken(st.session_token, st.time, st.secret_key);
        request->Init(&requester_);

        locker_->AddInProgressRequest(std::move(request), std::move(st));
        ATTEMPT_REQUEST_DEBUG("Started session token request.\n")
    }

    // Request session tokens if needed. Do not force the request or we could
    // get locked out of the account.
    RequestNeededSessionTokens(BadCredentialBehavior::NoForce);

    ATTEMPT_REQUEST_DEBUG("-- END --\n")
}
#undef ATTEMPT_REQUEST_DEBUG

void SessionMaintainer::AttemptConnection()
{
    auto maintainer_exists = maintainer_exists_;

#if defined(OUTPUT_DEBUG) || defined(DEBUG_API_CONNECTION)
    std::cout << "SessionMaintainer: Checking connection by calling "
                 "system/get_status.\n";
#endif

    hl::HttpRequest::Pointer http_request = requester_.Call(
            system::get_status::Request(),
            [this, maintainer_exists](
                    const system::get_status::Response & response)
            {
                if (*maintainer_exists == Exists::Yes)
                    HandleCheckConnectionStatusResponse(response);
            },
            RequestStarted::No);
    http_request->Start();
}

void SessionMaintainer::HandleCheckConnectionStatusResponse(
        const system::get_status::Response & response)
{
#if defined(OUTPUT_DEBUG) || defined(DEBUG_API_CONNECTION)
    std::cout << "SessionMaintainer: system/get_status response: "
              << response.error_code.message() << "\n" << response.debug
              << std::endl;
#endif

    UpdateConnectionStateFromErrorCode(response.error_code);
    if (IsConnected(GetConnectionState()))
        AttemptRequests();
    // else Connection still down- Keep waiting
}

/// Requests as many session tokens as allowed by the SessionTokenLocker.
void SessionMaintainer::RequestNeededSessionTokens(
        BadCredentialBehavior badCredentialBehavior)
{
    const bool haveBadCredentials = IsCredentialsError(GetSessionState());
    if (!haveBadCredentials
        || badCredentialBehavior == BadCredentialBehavior::Force)
    {
        const boost::optional<Credentials> credentials(
                locker_->GetCredenials());

        if (credentials)
        {
            if (haveBadCredentials)
            {  // Request only one token
                TryRequestSessionToken(*credentials);
            }
            else
            {  // Request as many tokens as we need
                while (TryRequestSessionToken(*credentials))
                {
#ifdef OUTPUT_DEBUG
                    std::cout << BOOST_CURRENT_FUNCTION
                              << ": Requested session token.\n";
#endif
                }
            }
        }
        else
        {
#ifdef OUTPUT_DEBUG
            std::cout << BOOST_CURRENT_FUNCTION << ": No credentials.\n";
#endif
        }
    }
    // else We can safely do nothing when the state is CredentialsFailure
    //      because we know that we will keep retrying.
}

bool SessionMaintainer::TryRequestSessionToken(const Credentials & credentials)
{
    // Must obtain permission via the mutexed request count.
    if (!locker_->StartRequestSessionToken())
    {
        return false;
    }
    else
    {
#ifdef OUTPUT_DEBUG  // Debug code
        std::cout << "SessionMaintainer: Requesting new session token."
                  << std::endl;
#endif

        auto maintainer_exists = maintainer_exists_;

        hl::HttpRequest::Pointer http_request = requester_.Call(
                user::get_session_token::Request(credentials),
                [this, maintainer_exists, credentials](
                        const user::get_session_token::Response & response)
                {
                    //  If not running, then we have shut down and token counts
                    //  are irrelevant.
                    if (*maintainer_exists == Exists::Yes)
                        HandleSessionTokenResponse(response, credentials);
                },
                RequestStarted::No);

        // The timeout for these can be shorter.
        /** @note The following line was commented out because a shorter timeout
         * for session tokens will cause us to alternate between network down
         * and invalid credentials if the credentials are bad but the network is
         * also slow. I don't see any harm in leaving this timeout the same as
         * all others. -ZCM */
        // http_request->SetTimeout(15);

        http_request->Start();

        return true;
    }
}

void SessionMaintainer::HandleSessionTokenResponse(
        const user::get_session_token::Response & response,
        const Credentials & old_credentials)
{
    locker_->DecrementSessionTokenInProgressCount();

    SessionState session_state;
    uint32_t session_state_change_count;

    std::tie(session_state, session_state_change_count)
            = locker_->GetSessionState();

    UpdateConnectionStateFromErrorCode(response.error_code);

    if (response.error_code)
    {
#ifdef OUTPUT_DEBUG  // Debug code
        std::cout << "SessionMaintainer: Session token request failed: "
                  << response.error_code.message() << "\n" << response.debug
                  << std::endl;
#endif

        // Increment failure count
        locker_->IncrementFailureCount();

        if (IsInvalidCredentialsError(response.error_code))
        {
            // Fail and set timeout before retry.
            session_token_failure_timer_.expires_from_now(
                    boost::posix_time::milliseconds(
                            session_token_credential_failure_wait_timeout_ms));

            if (!IsUninitialized(session_state)
                && !IsCredentialsError(session_state))
            {  // State is valid.

                const boost::optional<Credentials> credentials(
                        locker_->GetCredenials());

                // Equality check ensures this isn't an old request
                if (credentials && mf::utils::AreVariantsEqual(old_credentials,
                                                               *credentials))
                {
                    session_state::CredentialsFailure new_state;
                    new_state.session_token_response = response;
                    new_state.error_code = response.error_code;

                    // If the pkey was sent, then the password may have been
                    // changed.  Let the class user figure this out if so.
                    if (!response.pkey)
                        new_state.pkey = *response.pkey;

                    // This async call used the same credentials we currently
                    // have!  This means the username and password is bad.
                    locker_->SetSessionStateSafe(new_state,
                                                 session_state_change_count);
                }
            }
        }
        else
        {  // Non-credential error
            bool accountIsLocked
                    = (response.error_code == errc::AccountTemporarilyLocked);

            uint32_t wait_time
                    = (accountIsLocked
                               ? session_token_account_locked_wait_timeout_ms
                               : session_token_failure_wait_timeout_ms);

            // Fail and set timeout before retry.
            session_token_failure_timer_.expires_from_now(
                    boost::posix_time::milliseconds(wait_time));

            if (locker_->GetFailureCount() >= skProlongedFailureThreshold
                || accountIsLocked)
            {
                // Can only proceed to ProlongedError from Initialized, but will
                // continue to re-emit ProlongedError as long as we keep getting
                // errors. The locker will only emit a state change if the
                // actual error changes. The reason we can only proceed to
                // ProlongedError from Initialized is that they are essentially
                // equivalent for all purposes, except that prolonged error
                // advises users that we have been stuck in Initialized for
                // a long time due to prolonged errors.
                if (IsInitialized(session_state)
                    || IsProlongedError(session_state))
                {
                    session_state::ProlongedError new_state;
                    new_state.session_token_response = response;
                    new_state.error_code = response.error_code;
                    locker_->SetSessionStateSafe(new_state,
                                                 session_state_change_count);
                }
            }
        }

        session_token_failure_timer_.async_wait(boost::bind(
                &SessionMaintainer::HandleSessionTokenFailureTimeout, this,
                boost::asio::placeholders::error));
    }
    else
    {
// Session token is good.

#ifdef OUTPUT_DEBUG  // Debug code
        std::cout << "SessionMaintainer: Session token request success."
                  << std::endl;
#endif
        const auto & data = *response.response_data;

        locker_->ResetFailureCount();

        SessionTokenData st = {
                data.session_token, response.pkey, data.time, data.secret_key};

        if (locker_->AddSessionToken(std::move(st), old_credentials))
        {
            // Always move to running if we have a good session token and we are
            // not uninitialized.
            if (!IsUninitialized(session_state) && !IsRunning(session_state))
            {
                locker_->SetSessionStateSafe(session_state::Running{response},
                                             session_state_change_count);
            }
        }
        else
        {
#ifdef OUTPUT_DEBUG
            std::cout << "SessionMaintainer: Adding session token failed."
                      << std::endl;
#endif
        }

        AttemptRequests();
    }
#ifdef OUTPUT_DEBUG
    std::cout << "SessionMaintainer: " << GetSessionState() << std::endl;
#endif
}

void SessionMaintainer::HandleSessionTokenFailureTimeout(
        const boost::system::error_code & err)
{
    if (!err)
    {
        AttemptRequests();
        RequestNeededSessionTokens(BadCredentialBehavior::Force);
    }
}

void SessionMaintainer::HandleConnectionStateRecheckTimeout(
        const boost::system::error_code & err)
{
    if (!err)
    {
        AttemptRequests();
    }
}

void SessionMaintainer::AddWaitingRequest(STRequest request)
{
    locker_->AddWaitingRequest(request);
    AttemptRequests();
}

SessionState SessionMaintainer::GetSessionState()
{
    return locker_->GetSessionState().first;
}

void SessionMaintainer::SetSessionState(SessionState state)
{
    return locker_->SetSessionState(state);
}

ConnectionState SessionMaintainer::GetConnectionState()
{
    return locker_->GetConnectionState();
}

void SessionMaintainer::SetConnectionState(const ConnectionState & state)
{
    return locker_->SetConnectionState(state);
}

void SessionMaintainer::SetSessionStateChangeCallback(
        SessionStateChangeCallback callback)
{
    return locker_->SetSessionStateChangeCallback(callback);
}

void SessionMaintainer::SetConnectionStateChangeCallback(
        ConnectionStateChangeCallback callback)
{
    return locker_->SetConnectionStateChangeCallback(callback);
}

void SessionMaintainer::StopTimeouts() { locker_->StopTimeouts(); }

}  // namespace api
}  // namespace mf
