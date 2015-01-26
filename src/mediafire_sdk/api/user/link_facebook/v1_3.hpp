/**
 * @file api/user/link_facebook.hpp
 * @brief API request: /api/1.3/user/link_facebook
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
/** API action path "user" */
namespace user {
/** API action "user/link_facebook" */
namespace link_facebook {
/** API path "/api/1.3/user/link_facebook" */
namespace v1_3 {

/**
 * @class Response
 * @brief Response from API request "user/link_facebook"
 */
class Response : public ResponseBase
{
public:
};

class Impl;

/**
 * @class Request
 * @brief Make API request "user/link_facebook"
 */
class Request
{
public:
    /**
     * API request "user/link_facebook"
     *
     * @param access_token API parameter "fb_access_token"
     */
    explicit Request(
            std::string access_token
        );

    // Remaining functions are for use by API library only. --------------------

    /** Requester/SessionMaintainer expected type. */
    typedef Response ResponseType;

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

}  // namespace link_facebook
}  // namespace user
}  // namespace api
}  // namespace mf
