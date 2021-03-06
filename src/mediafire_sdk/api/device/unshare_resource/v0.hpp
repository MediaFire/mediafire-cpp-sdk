/**
 * @file api/device/unshare_resource.hpp
 * @brief API request: /api/device/unshare_resource
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
/** API action path "device" */
namespace device {
/** API action "device/unshare_resource" */
namespace unshare_resource {
/** API path "/api/device/unshare_resource" */
namespace v0 {

/**
 * @class ResponseData
 * @brief Response data from API request "device/unshare_resource"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** API response field "response.device_revision" */
    uint32_t device_revision;
};

/**
 * @class Response
 * @brief Response from API request "device/unshare_resource"
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
 * @brief Make API request "device/unshare_resource"
 */
class Request
{
public:
    /**
     * API request "device/unshare_resource"
     *
     * @param resource API parameter "resource"
     */
    explicit Request(
            std::string resource
        );

    /**
     * Optional API parameter "contact"
     *
     * @param contact Set parameter "contact" in API request.
     */
    void SetContact(std::string contact);

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
}  // namespace v0

}  // namespace unshare_resource
}  // namespace device
}  // namespace api
}  // namespace mf
