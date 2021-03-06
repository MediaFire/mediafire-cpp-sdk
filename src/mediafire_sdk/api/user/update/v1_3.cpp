/**
 * @file update.cpp
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
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_3 = mf::api::user::update::v1_3;


namespace {
std::string AsString(const v1_3::Gender & value)
{
    if (value == v1_3::Gender::Male)
        return "male";
    if (value == v1_3::Gender::Female)
        return "female";
    if (value == v1_3::Gender::None)
        return "none";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_3::ReceiveNewsletter & value)
{
    if (value == v1_3::ReceiveNewsletter::No)
        return "no";
    if (value == v1_3::ReceiveNewsletter::Yes)
        return "yes";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_3::PrimaryUsage & value)
{
    if (value == v1_3::PrimaryUsage::None)
        return "none";
    if (value == v1_3::PrimaryUsage::Home)
        return "home";
    if (value == v1_3::PrimaryUsage::Work)
        return "work";
    if (value == v1_3::PrimaryUsage::School)
        return "school";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
namespace update {
namespace v1_3 {

const std::string api_path("/api/1.3/user/update");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    boost::optional<std::string> current_password_;
    boost::optional<std::string> subdomain_;
    boost::optional<std::string> company_name_;
    boost::optional<std::string> email_;
    boost::optional<std::string> password_;
    boost::optional<std::string> first_name_;
    boost::optional<std::string> last_name_;
    boost::optional<std::string> display_name_;
    boost::optional<std::string> birth_date_;
    boost::optional<Gender> gender_;
    boost::optional<std::string> website_;
    boost::optional<std::string> location_;
    boost::optional<ReceiveNewsletter> receive_newsletter_;
    boost::optional<PrimaryUsage> primary_usage_;
    boost::optional<std::string> timezone_;
    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * /* response */ ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Post;
    }
};

Impl::Impl()
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

    // Always have response_data on success.
    response->response_data = ResponseData();

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    if (current_password_)
        parts["current_password"] = *current_password_;
    if (subdomain_)
        parts["subdomain"] = *subdomain_;
    if (company_name_)
        parts["company_name"] = *company_name_;
    if (email_)
        parts["email"] = *email_;
    if (password_)
        parts["password"] = *password_;
    if (first_name_)
        parts["first_name"] = *first_name_;
    if (last_name_)
        parts["last_name"] = *last_name_;
    if (display_name_)
        parts["display_name"] = *display_name_;
    if (birth_date_)
        parts["birth_date"] = *birth_date_;
    if (gender_)
        parts["gender"] = AsString(*gender_);
    if (website_)
        parts["website"] = *website_;
    if (location_)
        parts["location"] = *location_;
    if (receive_newsletter_)
        parts["receive_newsletter"] = AsString(*receive_newsletter_);
    if (primary_usage_)
        parts["primary_usage"] = AsString(*primary_usage_);
    if (timezone_)
        parts["timezone"] = *timezone_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request() :
    impl_(new Impl())
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

void Request::SetCurrentPassword(std::string current_password)
{
    impl_->current_password_ = current_password;
}

void Request::SetSubdomain(std::string subdomain)
{
    impl_->subdomain_ = subdomain;
}

void Request::SetCompanyName(std::string company_name)
{
    impl_->company_name_ = company_name;
}

void Request::SetEmail(std::string email)
{
    impl_->email_ = email;
}

void Request::SetPassword(std::string password)
{
    impl_->password_ = password;
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

void Request::SetBirthDate(std::string birth_date)
{
    impl_->birth_date_ = birth_date;
}

void Request::SetGender(Gender gender)
{
    impl_->gender_ = gender;
}

void Request::SetWebsite(std::string website)
{
    impl_->website_ = website;
}

void Request::SetLocation(std::string location)
{
    impl_->location_ = location;
}

void Request::SetReceiveNewsletter(ReceiveNewsletter receive_newsletter)
{
    impl_->receive_newsletter_ = receive_newsletter;
}

void Request::SetPrimaryUsage(PrimaryUsage primary_usage)
{
    impl_->primary_usage_ = primary_usage;
}

void Request::SetTimezone(std::string timezone)
{
    impl_->timezone_ = timezone;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace update
}  // namespace user
}  // namespace api
}  // namespace mf
