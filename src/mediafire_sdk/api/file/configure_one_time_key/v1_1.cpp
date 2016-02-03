/**
 * @file configure_one_time_key.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v1_1.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_1 = mf::api::file::configure_one_time_key::v1_1;


namespace {
std::string AsString(const v1_1::NofifyOwnerByEmail & value)
{
    if (value == v1_1::NofifyOwnerByEmail::Yes)
        return "yes";
    if (value == v1_1::NofifyOwnerByEmail::No)
        return "no";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_1::BurnAfterUse & value)
{
    if (value == v1_1::BurnAfterUse::No)
        return "no";
    if (value == v1_1::BurnAfterUse::Yes)
        return "yes";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "file" */
namespace file {
namespace configure_one_time_key {
namespace v1_1 {

const std::string api_path("/api/1.1/file/configure_one_time_key");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    explicit Impl(
            std::string token
        );

    std::string token_;
    boost::optional<uint32_t> duration_minutes_;
    boost::optional<NofifyOwnerByEmail> email_notification_;
    boost::optional<std::string> success_callback_url_;
    boost::optional<std::string> error_callback_url_;
    boost::optional<std::string> bind_ip_;
    boost::optional<BurnAfterUse> burn_after_use_;
    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * response ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const override
    {
        return mf::api::RequestMethod::Post;
    }
};

Impl::Impl(
        std::string token
    ) :
    token_(token)
{
}

void Impl::BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * query_parts
    ) const
{
    *path = api_path + ".php";
}

void Impl::ParseResponse( Response * response )
{
    // This function uses return defines for readability and maintainability.
#   define return_error(error_type, error_message)                             \
    {                                                                          \
        SetError(response, error_type, error_message);                         \
        return;                                                                \
    }

    ResponseData response_data;

    // For uniformity for code generation with the other content parsers.
    ResponseData * response_data_ptr = &response_data;
    response_data_ptr->one_time_key_request_count = 0;

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.one_time_key_request_count",
            &response_data_ptr->one_time_key_request_count);

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["token"] = token_;
    if (duration_minutes_)
        parts["duration"] = AsString(*duration_minutes_);
    if (email_notification_)
        parts["email_notification"] = AsString(*email_notification_);
    if (success_callback_url_)
        parts["success_callback_url"] = *success_callback_url_;
    if (error_callback_url_)
        parts["error_callback_url"] = *error_callback_url_;
    if (bind_ip_)
        parts["bind_ip"] = *bind_ip_;
    if (burn_after_use_)
        parts["burn_after_use"] = AsString(*burn_after_use_);

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string token
    ) :
    impl_(new Impl(token))
{
}

void Request::SetCallback( CallbackType callback_function )
{
    impl_->SetCallback(callback_function);
}

void Request::HandleContent(
        const std::string & url,
        const mf::http::Headers & headers,
        const std::string & content
    )
{
    impl_->HandleContent(url, headers, content);
}

void Request::HandleError(
        const std::string & url,
        std::error_code ec,
        const std::string & error_string
    )
{
    impl_->HandleError(url, ec, error_string);
}

std::string Request::Url(const std::string & hostname) const
{
    return impl_->Url(hostname);
}

void Request::SetSessionToken(
        std::string session_token,
        std::string time,
        int secret_key
    )
{
    impl_->SetSessionToken(session_token, time, secret_key);
}

void Request::SetDurationMinutes(uint32_t duration_minutes)
{
    impl_->duration_minutes_ = duration_minutes;
}

void Request::SetEmailNotification(NofifyOwnerByEmail email_notification)
{
    impl_->email_notification_ = email_notification;
}

void Request::SetSuccessCallbackUrl(std::string success_callback_url)
{
    impl_->success_callback_url_ = success_callback_url;
}

void Request::SetErrorCallbackUrl(std::string error_callback_url)
{
    impl_->error_callback_url_ = error_callback_url;
}

void Request::SetBindIp(std::string bind_ip)
{
    impl_->bind_ip_ = bind_ip;
}

void Request::SetBurnAfterUse(BurnAfterUse burn_after_use)
{
    impl_->burn_after_use_ = burn_after_use;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_1
}  // namespace configure_one_time_key
}  // namespace file
}  // namespace api
}  // namespace mf
