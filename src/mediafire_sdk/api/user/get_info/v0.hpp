/**
 * @file api/user/get_info.hpp
 * @brief API request: /api/user/get_info
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
/** API action "user/get_info" */
namespace get_info {
/** API path "/api/user/get_info" */
namespace v0 {

enum class Gender
{
    /** API value "unknown" */
    Unknown,
    /** API value "male" */
    Male,
    /** API value "female" */
    Female
};

enum class AccountType
{
    /** API value "No" */
    Basic,
    /** API value "Yes" */
    Premium
};

enum class Validated
{
    /** API value "No" */
    NotValidated,
    /** API value "Yes" */
    Validated
};

enum class LimitExceeded
{
    /** API value "no" */
    WithinBounds,
    /** API value "yes" */
    Exceeded
};

enum class FacebookLinked
{
    /** API value "no" */
    Unlinked,
    /** API value "yes" */
    Linked
};

enum class TwitterLinked
{
    /** API value "no" */
    Unlinked,
    /** API value "yes" */
    Linked
};

/**
 * @class ResponseData
 * @brief Response data from API request "user/get_info"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        first_name(""),
        last_name(""),
        gender(Gender::Unknown),
        birth_date(boost::posix_time::not_a_date_time),
        created_datetime(boost::posix_time::not_a_date_time)
    {}
    /** API response field "response.user_info.ekey" */
    std::string ekey;

    /** API response field "response.user_info.email" */
    std::string email;

    /** API response field "response.user_info.first_name" */
    std::string first_name;

    /** API response field "response.user_info.last_name" */
    std::string last_name;

    /** API response field "response.user_info.display_name" */
    std::string display_name;

    /** API response field "response.user_info.gender" */
    Gender gender;

    /** API response field "response.user_info.birth_date" */
    boost::posix_time::ptime birth_date;

    /** API response field "response.user_info.location" */
    boost::optional<std::string> location;

    /** API response field "response.user_info.website" */
    boost::optional<std::string> website;

    /** API response field "response.user_info.premium" */
    AccountType account_type;

    /** API response field "response.user_info.bandwidth" */
    int64_t bandwidth;

    /** API response field "response.user_info.created" */
    boost::posix_time::ptime created_datetime;

    /** API response field "response.user_info.validated" */
    Validated validated;

    /** API response field "response.user_info.max_upload_size" */
    uint64_t max_upload_size;

    /** API response field "response.user_info.max_instant_upload_size" */
    uint64_t max_instant_upload_size;

    /** API response field "response.user_info.tos_accepted" */
    boost::optional<std::string> tos_accepted;

    /** API response field "response.user_info.used_storage_size" */
    uint64_t used_storage_size;

    /** API response field "response.user_info.base_storage" */
    uint64_t base_storage;

    /** API response field "response.user_info.bonus_storage" */
    uint64_t bonus_storage;

    /** API response field "response.user_info.storage_limit" */
    uint64_t storage_limit;

    /** API response field "response.user_info.storage_limit_exceeded" */
    LimitExceeded storage_limit_exceeded;

    /** API response field "response.user_info.options" */
    uint32_t options;

    /** API response field "response.user_info.facebook.facebook_id" */
    boost::optional<std::string> facebook_id;

    /** API response field "response.user_info.facebook.date_created" */
    boost::optional<std::string> facebook_date_created;

    /** API response field "response.user_info.facebook.facebook_url" */
    boost::optional<std::string> facebook_url;

    /** API response field "response.user_info.facebook.name" */
    boost::optional<std::string> facebook_name;

    /** API response field "response.user_info.facebook.firstname" */
    boost::optional<std::string> facebook_firstname;

    /** API response field "response.user_info.facebook.lastname" */
    boost::optional<std::string> facebook_lastname;

    /** API response field "response.user_info.facebook.hometown" */
    boost::optional<std::string> facebook_hometown;

    /** API response field "response.user_info.facebook.location" */
    boost::optional<std::string> facebook_location;

    /** API response field "response.user_info.facebook.i18n" */
    boost::optional<std::string> facebook_i18n;

    /** API response field "response.user_info.facebook.timezone" */
    boost::optional<int32_t> facebook_timezone;

    /** API response field "response.user_info.facebook.linked" */
    boost::optional<FacebookLinked> facebook_linked;

    /** API response field "response.user_info.twitter.twitter_id" */
    boost::optional<std::string> twitter_id;

    /** API response field "response.user_info.twitter.date_created" */
    boost::optional<std::string> twitter_date_created;

    /** API response field "response.user_info.twitter.screen_name" */
    boost::optional<std::string> twitter_screen_name;

    /** API response field "response.user_info.twitter.name" */
    boost::optional<std::string> twitter_name;

    /** API response field "response.user_info.twitter.i18n" */
    boost::optional<std::string> twitter_i18n;

    /** API response field "response.user_info.twitter.linked" */
    boost::optional<TwitterLinked> twitter_linked;
};

/**
 * @class Response
 * @brief Response from API request "user/get_info"
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
 * @brief Make API request "user/get_info"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Gender = Gender;
    using AccountType = AccountType;
    using Validated = Validated;
    using LimitExceeded = LimitExceeded;
    using FacebookLinked = FacebookLinked;
    using TwitterLinked = TwitterLinked;

    /**
     * API request "user/get_info"
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
}  // namespace v0

}  // namespace get_info
}  // namespace user
}  // namespace api
}  // namespace mf
