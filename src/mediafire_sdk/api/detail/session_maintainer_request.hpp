/**
 * @file session_maintainer_request.hpp
 * @author Herbert Jones
 * @brief Request wrapper for session token maintainer API requests.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "mediafire_sdk/api/detail/request_interface.hpp"
#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/requester.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "boost/bind.hpp"

#if ! defined(NDEBUG)
#   include "boost/atomic.hpp"
#endif

// #define OUTPUT_DEBUG

namespace mf {
namespace api {
namespace detail {

/** Create struct which is only well defined if X is member of passed in class.
 */
#define CREATE_HAS_MEMBER(X)                                                   \
template<typename T> class Has##X                                              \
{                                                                              \
    struct Fallback { int X; };                                                \
    struct Derived : T, Fallback { };                                          \
    /* blank */                                                                \
    template<typename U, U> struct Check;                                      \
    /* blank */                                                                \
    typedef char ArrayOfOne[1];                                                \
    typedef char ArrayOfTwo[2];                                                \
    /* blank */                                                                \
    template<typename U> static ArrayOfOne &                                   \
    func(Check<int Fallback::*, &U::X> *);                                     \
    template<typename U> static ArrayOfTwo & func(...);                        \
public:                                                                        \
    typedef Has##X type;                                                       \
    enum { value = sizeof(func<Derived>(0)) == 2 };                            \
};
/** HasSetSessionToken<T>::value is set if T has method SetSessionToken */
CREATE_HAS_MEMBER(SetSessionToken);
#undef CREATE_HAS_MEMBER

class STRequestInterface : public RequestInterface
{
public:
    virtual void Cancel() = 0;
    virtual void Fail(
            const std::error_code & error_code,
            const std::string & error_string
        ) = 0;
    virtual void Init(Requester * requester) = 0;
    virtual void SetSessionToken(
            const std::string & session_token,
            const std::string & time,
            int secret_key
        ) = 0;
    virtual bool UsesSessionToken() const = 0;
    virtual uint32_t TimeoutSeconds() const = 0;

    virtual ~STRequestInterface() {}

private:
};

template<typename ApiFunctor>
typename std::enable_if<HasSetSessionToken<ApiFunctor>::value, void>::type
SetSessionTokenT(
        ApiFunctor * af,
        const std::string & session_token,
        const std::string & time,
        int secret_key
    )
{
    af->SetSessionToken(
            session_token,
            time,
            secret_key
        );
}

template<typename ApiFunctor>
typename std::enable_if< ! HasSetSessionToken<ApiFunctor>::value, void>::type
SetSessionTokenT(
        ApiFunctor * /* af */,
        const std::string & /* session_token */,
        const std::string & /* time */,
        int /* secret_key */
    )
{
    // Do nothing.
}

template<typename ApiFunctor>
typename std::enable_if<HasSetSessionToken<ApiFunctor>::value, bool>::type
UsesSessionTokenT()
{
    return true;
}

template<typename ApiFunctor>
typename std::enable_if< ! HasSetSessionToken<ApiFunctor>::value, bool>::type
UsesSessionTokenT()
{
    return false;
}

typedef std::shared_ptr<STRequestInterface> STRequest;
typedef std::weak_ptr<STRequestInterface> STRequestWeak;

#if ! defined(NDEBUG)
// Counter to keep track of number of SessionMaintainerRequests
extern boost::atomic<int> session_maintainer_request_count;
#endif

template<typename ApiFunctor, typename IoService>
class SessionMaintainerRequest :
    public std::enable_shared_from_this<
        SessionMaintainerRequest<ApiFunctor, IoService>>,
    public STRequestInterface
{
public:
    typedef SessionMaintainerRequest<ApiFunctor, IoService> SelfType;
    typedef typename ApiFunctor::ResponseType ResponseType;

    typedef std::shared_ptr< SelfType > Pointer;
    typedef std::weak_ptr< SelfType > WeakPointer;

    typedef std::function<void(STRequest, ResponseBase*)> CompletionCallback;

    static Pointer Create(
            ApiFunctor * af,
            typename ApiFunctor::CallbackType cb,
            IoService * callback_io_service,
            uint32_t timeout_seconds,
            CompletionCallback completion_alert,
            CompletionCallback request_retry_alert,
            CompletionCallback completion_notification
        )
    {
        Pointer request( new SelfType(af) );

        request->completion_callback_ = cb;
        request->callback_io_service_ = callback_io_service;
        request->timeout_seconds_ = timeout_seconds;
        request->max_retry_time_ =
            boost::posix_time::second_clock::universal_time() +
            boost::posix_time::seconds(timeout_seconds);
        request->completion_alert_ = completion_alert;
        request->request_retry_alert_ = request_retry_alert;
        request->completion_notification_ = completion_notification;

        return request;
    }

    virtual ~SessionMaintainerRequest()
    {
#if ! defined(NDEBUG)
        const int object_count = session_maintainer_request_count.fetch_sub(1,
            boost::memory_order_release);
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "--SessionMaintainerRequest: " << (object_count-1) << std::endl;
#       endif
        assert(object_count > 0);
#endif
    }

    virtual void Cancel() override
    {
        if (request_)
        {
            request_->Cancel();
        }
        else
        {
            auto self = this->shared_from_this();
            callback_io_service_->dispatch(
                [this, self]()
                {
                    // No http request yet.  Must create the response.
                    ResponseType response;
                    response.error_code = make_error_code(
                        mf::http::http_error::Cancelled );
                    response.error_string = "Cancelled";
                    completion_callback_(response);
                });
        }
    }

    virtual void Fail(
            const std::error_code & error_code,
            const std::string & error_string
        ) override
    {
        if (request_)
        {
            request_->Fail(error_code, error_string);
        }
        else
        {
            auto self = this->shared_from_this();
            callback_io_service_->dispatch(
                [this, self, error_code, error_string]()
                {
                    // No http request yet.  Must create the response.
                    ResponseType response;
                    response.error_code = error_code;
                    response.error_string = error_string;
                    completion_callback_(response);
                });
        }
    }

    typename ApiFunctor::CallbackType GetCallbackProxy()
    {
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "Setting callback proxy." << std::endl;
#       endif

        // As the session maintainer manages the life of these requests, there
        // is no requirement to ensure that the life of the object matches the
        // life of the callback.  Only weak pointers bound to callbacks.
        WeakPointer weak_self = this->shared_from_this();

        typename ApiFunctor::CallbackType cb = [this, weak_self](
                typename ApiFunctor::ResponseType response
            )
        {
            Pointer self = weak_self.lock();
            if (self)
            {
                self->CallbackProxy(response);
            }
        };

        return cb;
    }

    void CallbackProxy( typename ApiFunctor::ResponseType response )
    {
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "In Callback Proxy." << std::endl;
#       endif

        assert( completion_callback_ );

        // Update connection status
        completion_notification_(this->shared_from_this(), &response);

        // response is the data from the API request. If the session token was bad,
        // then we need to get a new session token.
        if ( ! HasSetSessionToken<ApiFunctor>::value || ! response.error_code )
        {
            completion_alert_(this->shared_from_this(), &response);
            completion_callback_(response);
            return;
        }

        // Only retry within certain time range.
        if ( boost::posix_time::second_clock::universal_time()
            < max_retry_time_ )
        {
            // Certain errors should be retried.
            if ( IsInvalidSessionTokenError(response.error_code)
                || response.error_code ==
                    api::result_code::AsyncOperationInProgress )
            {
                request_retry_alert_(this->shared_from_this(), &response);
            }
            else // other error code
            {
                completion_alert_(this->shared_from_this(), &response);
                completion_callback_(response);
            }
        }
        else
        {
            // Send error
            completion_alert_(this->shared_from_this(), &response);
            completion_callback_(response);
        }
    }

    virtual void Init(Requester * requester) override
    {
        request_ = requester->Wrap( af_, GetCallbackProxy(),
            callback_io_service_ );

        mf::http::HttpRequest::Pointer http_request =
            request_->Init(RequestStarted::No);

        http_request->SetTimeout(timeout_seconds_);

        http_request->Start();
    }

    virtual void SetSessionToken(
            const std::string & session_token,
            const std::string & time,
            int secret_key
        ) override
    {
        SetSessionTokenT(
                &af_,
                session_token,
                time,
                secret_key
            );
    }

    virtual bool UsesSessionToken() const override
    {
        return UsesSessionTokenT<ApiFunctor>();
    }

    virtual uint32_t TimeoutSeconds() const override {return timeout_seconds_;}

private:
    ApiFunctor af_;

    typedef detail::RequesterImpl<ApiFunctor, IoService> ApiImpl;
    typename ApiFunctor::CallbackType completion_callback_;

    IoService * callback_io_service_;

    uint32_t timeout_seconds_;
    boost::posix_time::ptime max_retry_time_;

    CompletionCallback completion_alert_;
    CompletionCallback request_retry_alert_;
    CompletionCallback completion_notification_;

    std::shared_ptr<ApiImpl> request_;

    explicit SessionMaintainerRequest(ApiFunctor * af) :
        af_(*af)
    {
#if ! defined(NDEBUG)
        const int object_count = session_maintainer_request_count.fetch_add(1,
            boost::memory_order_relaxed);
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "++SessionMaintainerRequest: " << (object_count+1) << std::endl;
#       endif
        assert( object_count < 100000 );
#endif
    }
};

}  // namespace detail
}  // namespace api
}  // namespace mf
