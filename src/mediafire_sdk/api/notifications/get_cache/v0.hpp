/**
 * @file api/notifications/get_cache.hpp
 * @brief API request: /api/notifications/get_cache
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
/** API action path "notifications" */
namespace notifications {
/** API action "notifications/get_cache" */
namespace get_cache {
/** API path "/api/notifications/get_cache" */
namespace v0 {

enum class Viewable
{
    /** API value "0" */
    Hidden,
    /** API value "1" */
    Viewable
};

/**
 * @class ResponseData
 * @brief Response data from API request "notifications/get_cache"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    struct Notification
    {
        /** API response field "actor" */
        std::string actor;

        /** API response field "timestamp" */
        boost::posix_time::ptime timestamp;

        /** API response field "message" */
        std::string message;

        /** API response field "resource" */
        boost::optional<std::string> resource;

        /** API response field "viewable" */
        Viewable viewable;
    };
    /** API response field "response.notifications" */
    std::vector<Notification> notifications;

    /** API response field "response.num_older" */
    uint32_t number_older;
};

/**
 * @class Response
 * @brief Response from API request "notifications/get_cache"
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
 * @brief Make API request "notifications/get_cache"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Viewable = ::mf::api::notifications::get_cache::v0::Viewable;

    /**
     * API request "notifications/get_cache"
     */
    Request();

    /**
     * Optional API parameter "start"
     *
     * @param start Set parameter "start" in API request.
     */
    void SetStart(uint32_t start);

    /**
     * Optional API parameter "limit"
     *
     * @param limit Set parameter "limit" in API request.
     */
    void SetLimit(uint32_t limit);

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

}  // namespace get_cache
}  // namespace notifications
}  // namespace api
}  // namespace mf
