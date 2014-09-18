/**
 * @file get_patch.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "get_patch.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::device::get_patch::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "device" */
namespace device {
namespace get_patch {
namespace v0 {

const std::string api_path("/api/device/get_patch");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            std::string quickkey,
            uint32_t source_revision,
            uint32_t target_revision
        );

    std::string quickkey_;
    uint32_t source_revision_;
    uint32_t target_revision_;
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
        std::string quickkey,
        uint32_t source_revision,
        uint32_t target_revision
    ) :
    quickkey_(quickkey),
    source_revision_(source_revision),
    target_revision_(target_revision)
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
            "response.patch_link",
            &response->patch_link ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.patch_link\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.patch_hash",
            &response->patch_hash ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.patch_hash\"");

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["quick_key"] = quickkey_;
    parts["source_revision"] = AsString(source_revision_);
    parts["target_revision"] = AsString(target_revision_);

    std::string post_data = MakePost(api_path + ".php", parts);
    api_data_type_debug_ += " POST data: " + post_data + "\n";
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string quickkey,
        uint32_t source_revision,
        uint32_t target_revision
    ) :
    impl_(new Impl(quickkey, source_revision, target_revision))
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

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v0
}  // namespace get_patch
}  // namespace device
}  // namespace api
}  // namespace mf
