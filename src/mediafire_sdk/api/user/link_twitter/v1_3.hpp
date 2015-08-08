/**
 * @file api/user/link_twitter.hpp
 * @brief API request: /api/1.3/user/link_twitter
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
/** API action "user/link_twitter" */
namespace link_twitter {
/** API path "/api/1.3/user/link_twitter" */
namespace v1_3 {

/**
 * @class ResponseData
 * @brief Response data from API request "user/link_twitter"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
};

/**
 * @class Response
 * @brief Response from API request "user/link_twitter"
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
 * @brief Make API request "user/link_twitter"
 */
class Request
{
public:
    /**
     * API request "user/link_twitter"
     *
     * @param oauth_token API parameter "tw_oauth_token"
     * @param oauth_token_secret API parameter "tw_oauth_token_secret"
     */
    Request(
            std::string oauth_token,
            std::string oauth_token_secret
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

}  // namespace link_twitter
}  // namespace user
}  // namespace api
}  // namespace mf
