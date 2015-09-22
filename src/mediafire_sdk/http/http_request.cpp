/**
 * @file http_request.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "http_request.hpp"

#include "mediafire_sdk/http/detail/http_request_state_machine.hpp"

namespace hl = mf::http;
namespace asio = boost::asio;

#if ! defined(NDEBUG)
// Counter to keep track of number of HttpRequests
boost::atomic<int> hl::detail::HttpRequestMachine_::request_count_(0);
#endif

class hl::HttpRequestImpl
{
public:
    HttpRequestImpl(
            HttpConfig::ConstPointer http_config,
            std::shared_ptr<RequestResponseInterface> callback,
            asio::io_service * callback_ios,
            const std::string & url
        ) :
        callback_(callback),
        state_machine_(
            std::make_shared<detail::HttpRequestMachine>(
                detail::HttpRequestMachineConfig{
                    http_config,
                    url,
                    callback,
                    callback_ios
                } ) )
    {
    }

    template<typename Event>
    void ProcessEvent(Event evt)
    {
        // For thread safety.
        state_machine_->ProcessEvent(evt);
    }

private:
    std::shared_ptr<RequestResponseInterface> callback_;

    std::shared_ptr<detail::HttpRequestMachine> state_machine_;
};

hl::HttpRequest::HttpRequest(
        HttpConfig::ConstPointer http_config,
        const std::shared_ptr<RequestResponseInterface> & callback,
        asio::io_service * callback_ios,
        const std::string & url
    )
{
    impl_.reset(
        new HttpRequestImpl (
            http_config,
            callback,
            callback_ios,
            url ));
}

hl::HttpRequest::~HttpRequest()
{
}

hl::HttpRequest::Pointer hl::HttpRequest::Create(
        HttpConfig::ConstPointer http_config,
        std::shared_ptr<RequestResponseInterface> callback,
        boost::asio::io_service * callback_ios,
        const std::string & url
    )
{
    Pointer ptr(
            new HttpRequest(
                http_config,
                callback,
                callback_ios,
                url
        ));

    return ptr;
}

hl::HttpRequest::Pointer hl::HttpRequest::Create(
        HttpConfig::ConstPointer http_config,
        std::shared_ptr<RequestResponseInterface> callback,
        const std::string & url
    )
{
    return Create(
        http_config,
        callback,
        http_config->GetDefaultCallbackIoService(),
        url );
}

mf::http::HttpRequest::Pointer mf::http::HttpRequest::Create(
        HttpConfig::ConstPointer http_config,
        std::function<void(HttpRequest::CallbackResponse)> callback,
        asio::io_service * callback_ios,
        const std::string & url
    )
{
    class Handler :
        public std::enable_shared_from_this<Handler>,
        public mf::http::RequestResponseInterface
    {
    public:
        Handler(
                std::function<void(HttpRequest::CallbackResponse)> callback,
                asio::io_service * callback_ios
                ) :
            callback_(callback),
            callback_ios_(callback_ios)
        {
        }

        /**
         * @brief Called after response header is parsed with redirect directions
         * which are being followed.
         *
         * @param[in] raw_header The headers in plain text.
         * @param[in] headers Headers parsed into parts.
         * @param[in] new_url New request target.
         */
        virtual void RedirectHeaderReceived(
                const mf::http::Headers & /*headers*/,
                const mf::http::Url & new_url
            ) override
        {
            response_.redirects.push_back(new_url.full_url());
        }

        /**
         * @brief Called after response header is parsed.
         *
         * @param[in] headers Headers parsed into parts.
         */
        virtual void ResponseHeaderReceived(
                const mf::http::Headers & headers
            ) override
        {
            response_.headers = headers;
        }

        /**
         * @brief Called when content received.
         *
         * As content is streamed from the remote server, this is called with the
         * streamed content.
         *
         * @param[in] start_pos Where in the response content the buffer starts.
         * @param[in] buffer The streamed data from the remote server.
         */
        virtual void ResponseContentReceived(
                size_t /* start_pos */,
                std::shared_ptr<mf::http::BufferInterface> buffer
            ) override
        {
            response_.content.append(
                reinterpret_cast<const char*>(buffer->Data()), buffer->Size() );
        }

        /**
         * @brief Called when an error occurs. Completes the request.
         *
         * @param[in] error_code The error code of the error.
         * @param[in] error_text Long description of the error.
         */
        virtual void RequestResponseErrorEvent(
                std::error_code error_code,
                std::string error_text
            ) override
        {
            response_.error_code = error_code;
            response_.error_text = error_text;

            auto self = shared_from_this();
            callback_ios_->post([this, self]()
                {
                    callback_(response_);
                });
        }

        /**
         * @brief Called when the request is successful. Completes the request.
         */
        virtual void RequestResponseCompleteEvent() override
        {
            callback_(response_);
        }

    private:
        HttpRequest::CallbackResponse response_;
        std::function<void(HttpRequest::CallbackResponse)> callback_;
        asio::io_service * callback_ios_;
    };

    auto handler = std::make_shared<Handler>(callback, callback_ios);

    return mf::http::HttpRequest::Create(
        http_config,
        std::static_pointer_cast<
            mf::http::RequestResponseInterface>(handler),
        http_config->GetWorkIoService(),
        url);
}

mf::http::HttpRequest::Pointer mf::http::HttpRequest::Create(
        HttpConfig::ConstPointer http_config,
        std::function<void(HttpRequest::CallbackResponse)> callback,
        const std::string & url
    )
{
    return Create(
        http_config,
        callback,
        http_config->GetDefaultCallbackIoService(),
        url );
}

void hl::HttpRequest::SetPostData(
        SharedBuffer::Pointer raw_data
    )
{
    impl_->ProcessEvent(
            detail::ConfigEvent{
                detail::ConfigEvent::ConfigPostData{raw_data}
        });
}

void hl::HttpRequest::SetPostDataPipe(
        std::shared_ptr<PostDataPipeInterface> pdi
    )
{
    impl_->ProcessEvent(
            detail::ConfigEvent{
                detail::ConfigEvent::ConfigPostDataPipe{pdi}
        });
}

void hl::HttpRequest::SetHeader(
        std::string name,
        std::string value
    )
{
    impl_->ProcessEvent(
            detail::ConfigEvent{
                detail::ConfigEvent::ConfigHeader{name, value}
        });
}

void hl::HttpRequest::SetRequestMethod(
        std::string method
    )
{
    impl_->ProcessEvent(
            detail::ConfigEvent{
                detail::ConfigEvent::ConfigRequestMethod{method}
        });
}

void hl::HttpRequest::Start()
{
    impl_->ProcessEvent(detail::StartEvent());
}

void hl::HttpRequest::Cancel()
{
    // Could be called from separate thread as this can only be called while
    // already running.
    impl_->ProcessEvent(detail::ErrorEvent{
            make_error_code( hl::http_error::Cancelled ),
            "Cancelled"
        });
}

void hl::HttpRequest::Fail(
        const std::error_code & error_code,
        const std::string & error_string
    )
{
    // Could be called from separate thread as this can only be called while
    // already running.
    impl_->ProcessEvent(detail::ErrorEvent{ error_code, error_string });
}

void hl::HttpRequest::SetTimeout(uint32_t timeout)
{
    impl_->ProcessEvent(
            detail::ConfigEvent{detail::ConfigEvent::ConfigTimeout{timeout}});
}

void hl::HttpRequest::SetRequestHeadersOnly()
{
    impl_->ProcessEvent(detail::ConfigEvent{
            detail::ConfigEvent::StopAfterReadingResponseHeaders{}});
}
