/**
 * @file http_request_events.hpp
 * @author Herbert Jones
 * @brief Events for the HttpRequest state machine.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <map>
#include <string>

#include "boost/asio.hpp"
#include "boost/variant/variant.hpp"

#include "mediafire_sdk/http/detail/types.hpp"
#include "mediafire_sdk/http/http_config.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/shared_buffer.hpp"

namespace mf
{
namespace http
{
namespace detail
{

// Setup events
struct ConfigEvent
{
    struct ConfigRedirectPolicy
    {
        RedirectPolicy redirect_policy;
    };
    struct ConfigRequestMethod
    {
        std::string method;
    };
    struct ConfigHeader
    {
        std::string name;
        std::string value;
    };
    struct ConfigPostDataPipe
    {
        std::shared_ptr<mf::http::PostDataPipeInterface> pdi;
    };
    struct ConfigPostData
    {
        SharedBuffer::Pointer raw_data;
    };
    struct ConfigTimeout
    {
        uint32_t timeout_seconds;
    };

    struct StopAfterReadingResponseHeaders
    {
    };

    typedef boost::variant<ConfigRedirectPolicy,
                           ConfigRequestMethod,
                           ConfigHeader,
                           ConfigPostDataPipe,
                           ConfigPostData,
                           ConfigTimeout,
                           StopAfterReadingResponseHeaders> ConfigVariant;

    ConfigVariant variant;
};

// Other external events
struct StartEvent {};
struct ErrorEvent
{
    std::error_code code;
    std::string description;
};

// Internal events
struct RestartEvent {};
struct InitializedEvent {};
struct ResolvedEvent
{
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
};
struct ConnectedEvent {};
struct HandshakeEvent {};
struct HeadersWrittenEvent {};
struct PostSent {};
struct HeadersReadEvent
{
    HeadersReadEvent() : status_code(0), content_length(0) {}

    std::string raw_headers;

    std::string http_version;
    uint16_t status_code;
    std::string status_message;

    uint64_t content_length;

    /**
     * HTTP header names are case insensitive, so the key value here is
     * converted to lowercase for ease of use.
     */
    std::map<std::string, std::string> headers;

    SharedStreamBuf read_buffer;
};
struct RedirectEvent : public HeadersReadEvent
{
    explicit RedirectEvent(const HeadersReadEvent&evt) :
        HeadersReadEvent(evt)
    {}

    std::string redirect_url;
};
struct RedirectedEvent {};
struct HeadersParsedEvent
{
    uint64_t content_length;

    /**
     * HTTP header names are case insensitive, so the key value here is
     * converted to lowercase for ease of use.
     */
    std::map<std::string, std::string> headers;

    SharedStreamBuf read_buffer;
};
struct ContentReadEvent {};

}  // namespace detail
}  // namespace http
}  // namespace mf
