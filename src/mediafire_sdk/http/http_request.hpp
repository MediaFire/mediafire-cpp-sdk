/**
 * @file http_request.hpp
 * @author Herbert Jones
 * @brief Frontend for HTTP communication state machine.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "boost/optional.hpp"
#include "boost/filesystem/path.hpp"

#include "mediafire_sdk/http/http_config.hpp"
#include "mediafire_sdk/http/request_response_interface.hpp"
#include "mediafire_sdk/http/shared_buffer.hpp"

#include "mediafire_sdk/utils/forward_declarations/asio.hpp"

namespace mf {
namespace http {

class HttpRequestImpl;
class PostDataPipeInterface;

/**
 * @class HttpRequest
 * @brief Simple way to request remote data and recieve the response via HTTP.
 */
class HttpRequest : public std::enable_shared_from_this<HttpRequest>
{
public:
    /**
     * @typedef Pointer
     * @brief Handle for HttpRequest objects.
     */
    typedef std::shared_ptr<HttpRequest> Pointer;

    /**
     * @struct Response
     * @brief HttpRequest callback response type.
     */
    struct CallbackResponse
    {
        /** Error set if error occurred. */
        std::error_code error_code;
        /** Possible long description of error if error occurred. */
        boost::optional<std::string> error_text;

        /** All redirects that occurred */
        std::vector<std::string> redirects;

        /** HTTP Response headers */
        Headers headers;

        /** HTTP Response content */
        std::string content;
    };

    using FunctionCallback = std::function<void(CallbackResponse)>;

    /**
     * @typedef HeaderContainer
     * @brief Headers to be used in request.
     */
    // Why not map? Allow order to be specified, plus we have to do a case
    // insensitive compare anyway.
    typedef std::vector< std::pair<std::string, std::string> > HeaderContainer;

    /**
     * @brief Http shared pointer constructor.
     *
     * @warning If you are using SSL, work_ios must be a single thread, or you
     *          must pass all S
     *
     * @throw boost::system::system_error If SSL certificate file can not be
     *        loaded if one was set.
     *
     * @param[in] config       HttpConfig with configuration options.
     * @param[in] callback     Object that receives request response.
     * @param[in] callback_ios Io_service where callback is done.
     * @param[in] url          The url to request.
     */
    static Pointer Create(
            HttpConfig::ConstPointer config,
            std::shared_ptr<RequestResponseInterface> callback,
            boost::asio::io_service * callback_ios,
            const std::string & url
        );

    /**
     * @brief Http shared pointer constructor.
     *
     * @warning If you are using SSL, work_ios must be a single thread, or you
     *          must pass all SSL calls to a single io_service. OpenSSL is not
     *          threadsafe.
     *
     * @throw boost::system::system_error If SSL certificate file can not be
     *        loaded if one was set.
     *
     * @param[in] config       HttpConfig with configuration options.
     * @param[in] callback     Object that receives request response.
     * @param[in] url          The url to request.
     */
    static Pointer Create(
            HttpConfig::ConstPointer config,
            std::shared_ptr<RequestResponseInterface> callback,
            const std::string & url
        );

    /**
     * @brief Http shared pointer constructor.
     *
     * This overload of Create uses a lambda instead of an interface.  Do not
     * use if the response is likely to contain a lot of data and should be
     * parsed little by little.
     *
     * @warning If you are using SSL, work_ios must be a single thread, or you
     *          must pass all SSL calls to a single io_service. OpenSSL is not
     *          threadsafe.
     *
     * @throw boost::system::system_error If SSL certificate file can not be
     *        loaded if one was set.
     *
     * @param[in] config       HttpConfig with configuration options.
     * @param[in] callback     Callback for request response.
     * @param[in] callback_ios Io_service where callback is done.
     * @param[in] url          The url to request.
     */
    static Pointer Create(
            HttpConfig::ConstPointer config,
            FunctionCallback callback,
            boost::asio::io_service * callback_ios,
            const std::string & url
        );

    /**
     * @brief Http shared pointer constructor.
     *
     * This overload of Create uses a lambda instead of an interface.  Do not
     * use if the response is likely to contain a lot of data and should be
     * parsed little by little.
     *
     * @warning If you are using SSL, work_ios must be a single thread, or you
     *          must pass all SSL calls to a single io_service. OpenSSL is not
     *          threadsafe.
     *
     * @throw boost::system::system_error If SSL certificate file can not be
     *        loaded if one was set.
     *
     * @param[in] config       HttpConfig with configuration options.
     * @param[in] callback     Callback for request response.
     * @param[in] url          The url to request.
     */
    static Pointer Create(
            HttpConfig::ConstPointer config,
            std::function<void(CallbackResponse)> callback,
            const std::string & url
        );

    ~HttpRequest();

    /**
     * @brief Sets POST data to be sent with request.
     * @warning Must be called before Start().
     *
     * Sets request method to POST and sets the "Content-Length" header field.
     * You can not use the SetPostDataPipe function if you set this. Uses a
     * shared pointer to prevent potential needless copying of large amounts of
     * data.
     *
     * @param[in] raw_data The data to be sent after the header.
     */
    void SetPostData( SharedBuffer::Pointer raw_data );

    /**
     * @brief Sets POST data to be sent with request, as a function.
     * @warning Must be called before Start().
     *
     * Sets request method to POST and sets the "Content-Length" header field
     * based on the contents of of the PostData object.  You can not use the
     * SetPostData function if you set this.
     *
     * @param[in] pdi The PostData object that can pipe post data as requested.
     */
    void SetPostDataPipe( std::shared_ptr<PostDataPipeInterface> pdi );

    /**
     * @brief Set a header field.
     * @warning Must be called before Start().
     *
     * @param[in] name  The header field name.
     * @param[in] value Data for the field.
     */
    void SetHeader( std::string name, std::string value );

    /**
     * @brief Set the HTTP request method.
     * @warning Must be called before Start().
     *
     * This can be used to use advanced HTTP features. If not used the default
     * GET is used.
     *
     * @param[in] method The request method, such as GET, HEAD, POST, PUT...
     */
    void SetRequestMethod( std::string method );

    /**
     * @brief Set the timeout seconds.
     *
     * The default timeout is 60 seconds.
     *
     * @param[in] timeout The new timeout in seconds.
     */
    void SetTimeout(uint32_t timeout);

    /**
     * @brief Set request to complete after reading the headers.
     */
    void SetRequestHeadersOnly();

    /**
     * @brief Start the request.
     *
     * This starts the request. No more operations should be done on the object
     * besides Cancel().
     */
    void Start();

    /**
     * @brief Cancel the request if possible.
     *
     * The request will be cancelled if not already complete.
     */
    void Cancel();

    /**
     * @brief Cancel the request if possible with custom failure.
     *
     * The request will be cancelled if not already complete.
     */
    void Fail(
            const std::error_code & error_code,
            const std::string & error_string
        );

private:
    /**
     * @brief Private implementation.
     * @note MSM is template heavy. We should isolate it to a single compilation unit.
     */
    std::unique_ptr<HttpRequestImpl> impl_;

    /**
     * See Create()
     */
    HttpRequest(
            HttpConfig::ConstPointer config,
            const std::shared_ptr<RequestResponseInterface> & callback,
            boost::asio::io_service * callback_ios,
            const std::string & url
        );

    //static ProxyList proxies_;
};

}  // namespace http
}  // namespace mf
