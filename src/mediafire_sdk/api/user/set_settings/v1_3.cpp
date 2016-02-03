/**
 * @file set_settings.cpp
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

namespace v1_3 = mf::api::user::set_settings::v1_3;


namespace {
std::string AsString(const v1_3::ShareLinkStatus & value)
{
    if (value == v1_3::ShareLinkStatus::Disabled)
        return "disabled";
    if (value == v1_3::ShareLinkStatus::Enabled)
        return "enabled";
    if (value == v1_3::ShareLinkStatus::Inherit)
        return "inherit";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_3::CollectMetadata & value)
{
    if (value == v1_3::CollectMetadata::No)
        return "no";
    if (value == v1_3::CollectMetadata::Yes)
        return "yes";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
namespace set_settings {
namespace v1_3 {

const std::string api_path("/api/1.3/user/set_settings");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    boost::optional<uint32_t> previous_file_versions_;
    boost::optional<ShareLinkStatus> default_share_link_status_;
    boost::optional<CollectMetadata> collect_metadata_;
    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * /* response */ ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const override
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

    if (previous_file_versions_)
        parts["previous_file_versions"] = AsString(*previous_file_versions_);
    if (default_share_link_status_)
        parts["default_share_link_status"] = AsString(*default_share_link_status_);
    if (collect_metadata_)
        parts["collect_metadata"] = AsString(*collect_metadata_);

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

void Request::SetPreviousFileVersions(uint32_t previous_file_versions)
{
    impl_->previous_file_versions_ = previous_file_versions;
}

void Request::SetDefaultShareLinkStatus(ShareLinkStatus default_share_link_status)
{
    impl_->default_share_link_status_ = default_share_link_status;
}

void Request::SetCollectMetadata(CollectMetadata collect_metadata)
{
    impl_->collect_metadata_ = collect_metadata;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace set_settings
}  // namespace user
}  // namespace api
}  // namespace mf
