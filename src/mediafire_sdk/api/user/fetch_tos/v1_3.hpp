/**
 * @file api/user/fetch_tos.hpp
 * @brief API request: /api/1.3/user/fetch_tos
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
/** API action "user/fetch_tos" */
namespace fetch_tos {
/** API path "/api/1.3/user/fetch_tos" */
namespace v1_3 {

enum class TOSAccepted
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

/**
 * @class ResponseData
 * @brief Response data from API request "user/fetch_tos"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        user_accepted_terms(TOSAccepted::No)
    {}
    /** The version of the Terms of Service. */
    std::string revision;

    /** The complete Terms of Service. */
    std::string terms;

    /** The date and time the Terms of Service was created. */
    boost::posix_time::ptime date;

    /** Whether user accepted terms previously. */
    TOSAccepted user_accepted_terms;

    /** Token to be used with user/accept_tos. */
    std::string acceptance_token;
};

/**
 * @class Response
 * @brief Response from API request "user/fetch_tos"
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
 * @brief Make API request "user/fetch_tos"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using TOSAccepted = ::mf::api::user::fetch_tos::v1_3::TOSAccepted;

    /**
     * API request "user/fetch_tos"
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

}  // namespace fetch_tos
}  // namespace user
}  // namespace api
}  // namespace mf
