/**
 * @file user_register.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v1_3.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/tokenless_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_3 = mf::api::user::user_register::v1_3;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
namespace user_register {
namespace v1_3 {

const std::string api_path("/api/1.3/user/register");

// Impl ------------------------------------------------------------------------

class Impl : public TokenlessApiBase<Response>
{
public:
    explicit Impl(
            std::string application_id
        );

    std::string application_id_;
    boost::optional<std::string> email_;
    boost::optional<std::string> password_;
    boost::optional<std::string> fb_access_token_;
    boost::optional<std::string> first_name_;
    boost::optional<std::string> last_name_;
    boost::optional<std::string> display_name_;
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
        std::string application_id
    ) :
    application_id_(application_id)
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

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.email",
            &response->email ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.email\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.created",
            &response->created_datetime ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.created\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.pkey",
            &response->pkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.pkey\"");

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.temp_pw",
                &optarg) )
        {
            response->temporary_password = optarg;
        }
    }

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["application_id"] = application_id_;
    if (email_)
        parts["email"] = *email_;
    if (password_)
        parts["password"] = *password_;
    if (fb_access_token_)
        parts["fb_access_token"] = *fb_access_token_;
    if (first_name_)
        parts["first_name"] = *first_name_;
    if (last_name_)
        parts["last_name"] = *last_name_;
    if (display_name_)
        parts["display_name"] = *display_name_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string application_id
    ) :
    impl_(new Impl(application_id))
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

void Request::SetEmail(std::string email)
{
    impl_->email_ = email;
}

void Request::SetPassword(std::string password)
{
    impl_->password_ = password;
}

void Request::SetFbAccessToken(std::string fb_access_token)
{
    impl_->fb_access_token_ = fb_access_token;
}

void Request::SetFirstName(std::string first_name)
{
    impl_->first_name_ = first_name;
}

void Request::SetLastName(std::string last_name)
{
    impl_->last_name_ = last_name;
}

void Request::SetDisplayName(std::string display_name)
{
    impl_->display_name_ = display_name;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace user_register
}  // namespace user
}  // namespace api
}  // namespace mf
