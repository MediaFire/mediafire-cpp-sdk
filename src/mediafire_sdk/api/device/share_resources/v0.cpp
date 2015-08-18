/**
 * @file share_resources.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v0.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::device::share_resources::v0;


namespace {
std::string AsString(const v0::Permissions & value)
{
    if (value == v0::Permissions::Read)
        return "1";
    if (value == v0::Permissions::ReadWrite)
        return "2";
    if (value == v0::Permissions::Manage)
        return "4";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "device" */
namespace device {
namespace share_resources {
namespace v0 {

const std::string api_path("/api/device/share_resources");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            std::string resources,
            std::string contacts
        );

    std::string resources_;
    std::string contacts_;
    boost::optional<Permissions> permissions_;
    boost::optional<std::string> message_;
    boost::optional<std::string> fb_access_token_;
    boost::optional<std::string> tw_oauth_token_;
    boost::optional<std::string> tw_oauth_token_secret_;
    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * response ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Post;
    }
};

Impl::Impl(
        std::string resources,
        std::string contacts
    ) :
    resources_(resources),
    contacts_(contacts)
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

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.device_revision",
            &response_data_ptr->device_revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.device_revision\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["resources"] = resources_;
    parts["contacts"] = contacts_;
    if (permissions_)
        parts["permissions"] = AsString(*permissions_);
    if (message_)
        parts["message"] = *message_;
    if (fb_access_token_)
        parts["fb_access_token"] = *fb_access_token_;
    if (tw_oauth_token_)
        parts["tw_oauth_token"] = *tw_oauth_token_;
    if (tw_oauth_token_secret_)
        parts["tw_oauth_token_secret"] = *tw_oauth_token_secret_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string resources,
        std::string contacts
    ) :
    impl_(new Impl(resources, contacts))
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

void Request::SetPermissions(Permissions permissions)
{
    impl_->permissions_ = permissions;
}

void Request::SetMessage(std::string message)
{
    impl_->message_ = message;
}

void Request::SetFbAccessToken(std::string fb_access_token)
{
    impl_->fb_access_token_ = fb_access_token;
}

void Request::SetTwOauthToken(std::string tw_oauth_token)
{
    impl_->tw_oauth_token_ = tw_oauth_token;
}

void Request::SetTwOauthTokenSecret(std::string tw_oauth_token_secret)
{
    impl_->tw_oauth_token_secret_ = tw_oauth_token_secret;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v0
}  // namespace share_resources
}  // namespace device
}  // namespace api
}  // namespace mf
