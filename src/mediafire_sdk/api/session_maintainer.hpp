/**
 * @file session_maintainer.hpp
 * @author Herbert Jones
 * @brief Class to maintain a list of session tokens.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <functional>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "boost/asio/steady_timer.hpp"

#include "mediafire_sdk/api/connection_state.hpp"
#include "mediafire_sdk/api/credentials.hpp"
#include "mediafire_sdk/api/detail/session_maintainer_request.hpp"
#include "mediafire_sdk/api/session_state.hpp"
#include "mediafire_sdk/api/system/get_status.hpp"
#include "mediafire_sdk/api/user/get_session_token.hpp"

namespace mf {
namespace api {

namespace detail {
class SessionMaintainerLocker;
}  // namespace detail

/**
 * @class SessionMaintainer
 *
 * @warning If the session maintain is destroyed before all ongoing API requests
 * are complete, those requests will be canceled.  Their callbacks may be called
 * after the destruction of the maintainer.
 *
 * @warning If placing long running tasks in callbacks or on the work
 * io_service, http requests can time out, as they won't be handled before the
 * connection times out.  If you believe this may happen due to any long running
 * processes, then use a separate thread to run the io_service.
 *
 * @brief Maintains a list of session tokens, which are required to make most
 * API requests and keeps status of obtaining those tokens.
 */
class SessionMaintainer
{
public:
    /** Handle to an API request. */
    using Request = std::shared_ptr<detail::RequestInterface>;

    /** Class static constants */
    enum {
        max_in_progress_token_requests = 4,
        max_tokens = 10,
        session_token_failure_wait_timeout_ms = 2500,
        connection_state_recheck_timeout_ms = 5000,

        /**
         * Time to wait between session token requests during credential
         * failure. We have to throttle attempts to prevent locking account.
         */
        session_token_credential_failure_wait_timeout_ms = 60000,

        /**
         * Time to wait between requests when account is locked. We have to
         * give it time or we will constantly reset the locked out timer and
         * the user will never be able to log in.
         */
         session_token_account_locked_wait_timeout_ms = 600000,
    };

    /** Signature for session state change callbacks */
    typedef std::function<void(api::SessionState)> SessionStateChangeCallback;

    /** Signature for connection state change callbacks */
    typedef std::function<void(api::ConnectionState)>
        ConnectionStateChangeCallback;

    /**
     * @brief Create the session token maintainer which will make API calls to
     * www.mediafire.com.
     *
     * @param[in] http_config HttpConfig object
     */
    SessionMaintainer(
            mf::http::HttpConfig::ConstPointer http_config
        );

    /**
     * @brief Create the session token maintainer.
     *
     * @param[in] http_config HttpConfig object
     * @param[in] hostname Hostname to use for API requests.
     */
    SessionMaintainer(
            mf::http::HttpConfig::ConstPointer http_config,
            std::string hostname
        );

    ~SessionMaintainer();

    /**
     * @brief Set the callback for session state changes.
     *
     * The callback will be handled by the callback io_service.
     *
     * @param[in] callback Function to call on change.
     */
    void SetSessionStateChangeCallback(
            SessionStateChangeCallback callback
        );

    /**
     * @brief Set the callback for connection state changes.
     *
     * The callback will be handled by the callback io_service.
     *
     * @param[in] callback Function to call on change.
     */
    void SetConnectionStateChangeCallback(
            ConnectionStateChangeCallback callback
        );

    /**
     * @brief Set the credentials.
     *
     * The credentials must be set before any calls that require a session token
     * can be performed.
     *
     * @param[in] credentials The credentials for the account.
     */
    void SetLoginCredentials(
            const Credentials & credentials
        );

    /**
     * @brief Make an API call.
     *
     * @param[in] api_container The API object which holds all the data with
     * which to make the call.
     * @param[in] callback Function called when the API request completes.
     * @param[in] ios IO service where callback will be performed.
     *
     * @return API request object
     */
    template<typename ApiFunctor, typename IoService>
    Request Call(
            ApiFunctor api_container,
            typename ApiFunctor::CallbackType callback,
            IoService * callback_io_service
        )
    {
        detail::STRequest request =
            detail::SessionMaintainerRequest<ApiFunctor, IoService>::Create(
                &api_container,
                callback,
                callback_io_service,
                timeout_seconds_,
                on_stop_request_callback_,
                on_retry_request_callback_,
                info_update_callback_ );

        // SessionMaintainer maintain the life of request.
        AddWaitingRequest(request);

        return request;
    }

    /**
     * @brief Make an API call.
     *
     * The callback will be performed in the work thread passed in during
     * creation of the SessionMaintainer.
     *
     * @param[in] api_container The API object which holds all the data with
     * which to make the call.
     * @param[in] callback Function called when the API request completes.
     *
     * @return API request object
     */
    template<typename ApiFunctor>
    Request Call(
            ApiFunctor api_container,
            typename ApiFunctor::CallbackType callback
        )
    {
        return Call( api_container, callback,
            http_config_->GetDefaultCallbackIoService());
    }

    /**
     * @brief Set the normal number of timeout seconds.
     *
     * This sets the HTTP connect and retry timeout duration. Certain
     * error codes are retried automatically. This timeout prevents the request
     * from trying infinitely for such errors.
     *
     * @param[in] timeout_seconds Seconds till request timeout.
     */
    void SetTimeoutSeconds(uint32_t timeout_seconds)
    {
        timeout_seconds_ = timeout_seconds;
    }

    /**
     * @brief Stop all io_service timeouts so io_service::run can return as soon
     * as there is no longer any work to perform.
     *
     * This is mostly useful for unit tests.
     */
    void StopTimeouts();

    /**
     * @brief Get the HttpConfig object associated with the maintainer.
     *
     * @return HttpConfig used for all configuration.
     */
    mf::http::HttpConfig::ConstPointer HttpConfig() const {return http_config_;}

private:
    using CompletionCallback = std::function<void(detail::STRequest,
        ResponseBase*)>;

    // This is called when the request is completed successfully or not.
    CompletionCallback on_stop_request_callback_;

    // This is called when the request fails and can be retried.
    CompletionCallback on_retry_request_callback_;

    // Always called so connection status, ect. can be updated.
    CompletionCallback info_update_callback_;

    std::unique_ptr<detail::SessionMaintainerLocker> locker_;

    mf::http::HttpConfig::ConstPointer http_config_;
    boost::asio::deadline_timer session_token_failure_timer_;
    boost::asio::steady_timer connection_state_recheck_timer_;

    Requester requester_;

    uint32_t timeout_seconds_;

    enum class Exists
    {
        Yes,
        No
    };
    /** This does the same job as a weak pointer would, but this class doesn't
        enforce being shared. */
    std::shared_ptr<Exists> maintainer_exists_;

    api::SessionState GetSessionState();
    void SetSessionState(api::SessionState);

    api::ConnectionState GetConnectionState();
    void SetConnectionState(const api::ConnectionState &);

    void HandleCompletion(
            detail::STRequest request,
            ResponseBase * response
        );

    void HandleRetryRequest(
            detail::STRequest request,
            ResponseBase * response
        );
    void HandleCompletionNotification(
            detail::STRequest request,
            ResponseBase * response);

    void UpdateConnectionStateFromErrorCode(const std::error_code &);

    void AttemptRequests();
    void AttemptConnection();

    /**
     * @brief Try to request a session token if there are not already too many
     *        in progress session token requests in progress.
     *
     * @return True if request began.
     */
    bool TryRequestSessionToken(const Credentials & credentials);

    enum class BadCredentialBehavior
    {
        Force,
        NoForce,
    };
    void RequestNeededSessionTokens(
            BadCredentialBehavior badCredentialBehavior
        );

    void HandleSessionTokenResponse(
            const api::user::get_session_token::Response & response,
            const Credentials & credentials
        );

    void HandleCheckConnectionStatusResponse(
            const api::system::get_status::Response& response
        );

    void HandleDelayedRequestTimeout( detail::STRequest );

    void HandleSessionTokenFailureTimeout(
            const boost::system::error_code & err
        );
    void HandleConnectionStateRecheckTimeout(
            const boost::system::error_code & err
        );

    void AddWaitingRequest( detail::STRequest request );
};

}  // namespace api
}  // namespace mf
