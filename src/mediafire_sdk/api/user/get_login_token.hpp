/**
 * @file get_login_token.hpp
 * @author Herbert Jones
 * @brief API user/get_login_token
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <functional>

#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"
#include "mediafire_sdk/api/credentials.hpp"

namespace mf
{
namespace api
{
namespace user
{
namespace get_login_token
{
namespace v0
{

/**
 * @class ResponseData
 * @brief ResponseData from API request "user/get_login_token"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** Token used to log into user's account. */
    std::string login_token;
};

/**
 * @class Response
 * @brief Response from API request "user/get_login_token"
 */
class Response : public ResponseBase
{
public:
    /** Parsed API response on successful parse. */
    boost::optional<ResponseData> response_data;

    /** Unique password identifier.  Changes when password changes. */
    boost::optional<std::string> pkey;
};

/**
 * Api user/get_login_token. To be used with Requester.
 */
class Request
{
public:
    /** Requester/SessionMaintainer expected type. */
    typedef Response ResponseType;

    /** Callback type */
    typedef std::function<void(const ResponseType & data)> CallbackType;

    /** Requester/SessionMaintainer expected method. */
    void SetCallback(CallbackType callback_function)
    {
        callback_ = callback_function;
    };

    /**
     * @brief CTOR
     *
     * @param[in] credentials  API credentials for login token
     */
    Request(const Credentials & credentials);

    /** Requester expected method. */
    std::string Url(std::string hostname) const;

    /** Requester expected method. */
    void HandleContent(const std::string & url,
                       const mf::http::Headers & headers,
                       const std::string & content);

    /** Requester expected method. */
    void HandleError(const std::string & url,
                     std::error_code ec,
                     const std::string & error_string);

private:
    const Credentials credentials_;

    CallbackType callback_;
};
}  // namespace v0

// The latest version
using namespace v0;  // NOLINT

}  // namespace get_login_token
}  // namespace user
}  // namespace api
}  // namespace mf
