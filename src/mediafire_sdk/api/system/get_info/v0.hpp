/**
 * @file api/system/get_info.hpp
 * @brief API request: /api/system/get_info
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
#pragma once

#include <string>
#include <vector>

#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace api {
/** API action path "system" */
namespace system {
/** API action "system/get_info" */
namespace get_info {
/** API path "/api/system/get_info" */
namespace v0 {

/**
 * @class ResponseData
 * @brief Response data from API request "system/get_info"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    struct ImageSize
    {
        /** API response field "width" */
        uint32_t width;

        /** API response field "height" */
        uint32_t height;

        /** API response field "thumbnail" */
        std::string thumbnail_designation;
    };
    struct TermsOfService
    {
        /** API response field "revision" */
        std::string revision;

        /** API response field "terms" */
        std::string terms;

        /** API response field "date" */
        boost::posix_time::ptime date;
    };
    /** API response field "response.current_api_version" */
    std::string current_api_version;

    /** API response field "response.timezone" */
    std::string timezone;

    /** API response field "response.timezone_offset" */
    int32_t timezone_offset;

    /** API response field "response.viewable.extensions" */
    std::vector<std::string> viewable_extensions;

    /** API response field "response.editable.extensions" */
    std::vector<std::string> editable_extensions;

    /** API response field "response.image_sizes" */
    std::vector<ImageSize> image_sizes;

    /** API response field "response.terms_of_service" */
    TermsOfService terms_of_service;

    /** API response field "response.max_keys" */
    uint32_t max_keys;

    /** API response field "response.max_objects" */
    uint32_t max_objects;

    /** API response field "response.max_image_size" */
    uint64_t max_image_size;
};

/**
 * @class Response
 * @brief Response from API request "system/get_info"
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
 * @brief Make API request "system/get_info"
 */
class Request
{
public:
    /**
     * API request "system/get_info"
     */
    Request();

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

private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v0

}  // namespace get_info
}  // namespace system
}  // namespace api
}  // namespace mf
