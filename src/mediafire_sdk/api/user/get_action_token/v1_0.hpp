/**
 * @file api/user/get_action_token.hpp
 * @brief API request: /api/1.0/user/get_action_token
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
/** API action "user/get_action_token" */
namespace get_action_token {
/** API path "/api/1.0/user/get_action_token" */
namespace v1_0 {

enum class Type
{
    /** API value "image" */
    Image,
    /** API value "upload" */
    Upload
};

/**
 * @class ResponseData
 * @brief Response data from API request "user/get_action_token"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** API response field "response.action_token" */
    std::string action_token;
};

/**
 * @class Response
 * @brief Response from API request "user/get_action_token"
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
 * @brief Make API request "user/get_action_token"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Type = ::mf::api::user::get_action_token::v1_0::Type;

    /**
     * API request "user/get_action_token"
     *
     * @param type Action token type(image or upload)
     */
    explicit Request(
            Type type
        );

    /**
     * Optional API parameter "lifespan"
     *
     * @param lifespan The number of minutes before the action token should be
     *                 destroyed.  Defaults to 1 minute.
     */
    void SetLifespan(uint32_t lifespan);

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
}  // namespace v1_0

}  // namespace get_action_token
}  // namespace user
}  // namespace api
}  // namespace mf
