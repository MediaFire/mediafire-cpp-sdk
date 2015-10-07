/**
 * @file api/file/restore.hpp
 * @brief API request: /api/1.3/file/restore
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
#pragma once

#include <string>
#include <vector>

#include "mediafire_sdk/http/shared_buffer.hpp"
#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace api {
/** API action path "file" */
namespace file {
/** API action "file/restore" */
namespace restore {
/** API path "/api/1.3/file/restore" */
namespace v1_3 {

enum class Asynchronous
{
    /** API value "no" */
    Synchronous,
    /** API value "yes" */
    Asynchronous
};

/**
 * @class ResponseData
 * @brief Response data from API request "file/restore"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        asynchronous(Asynchronous::Synchronous)
    {}
    /** API response field "response.asynchronous" */
    Asynchronous asynchronous;
};

/**
 * @class Response
 * @brief Response from API request "file/restore"
 */
class Response : public ResponseBase
{
public:
    /** Parsed API response on successful parse. */
    boost::optional<ResponseData> response_data;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "file/restore"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Asynchronous = ::mf::api::file::restore::v1_3::Asynchronous;

    /**
     * API request "file/restore"
     *
     * @param quickkey API parameter "quick_key"
     * @param revision API parameter "revision"
     */
    Request(
            std::string quickkey,
            uint32_t revision
        );

    // Remaining functions are for use by API library only. --------------------

    /** Requester/SessionMaintainer expected type. */
    using ResponseType = Response;

    /** Requester/SessionMaintainer expected type. */
    using ResponseDataType = ResponseData;

    /** Requester/SessionMaintainer expected type. */
    typedef std::function< void( const ResponseType & data)> CallbackType;

    /** Requester/SessionMaintainer expected type. */
    void SetCallback( CallbackType callback_function );

    /** Requester expected method. */
    void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content
        );

    /** Requester expected method. */
    void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string
        );

    /** Requester expected method. */
    std::string Url(const std::string & hostname) const;

    /** Requester optional method. */
    mf::http::SharedBuffer::Pointer GetPostData();

    /** SessionMaintainer expected method. */
    void SetSessionToken(
            std::string session_token,
            std::string time,
            int secret_key
        );
private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v1_3

}  // namespace restore
}  // namespace file
}  // namespace api
}  // namespace mf
