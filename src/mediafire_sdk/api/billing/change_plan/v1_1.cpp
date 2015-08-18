/**
 * @file change_plan.cpp
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

namespace v1_1 = mf::api::billing::change_plan::v1_1;


namespace {
std::string AsString(const v1_1::InfoOnly & value)
{
    if (value == v1_1::InfoOnly::No)
        return "no";
    if (value == v1_1::InfoOnly::Yes)
        return "yes";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "billing" */
namespace billing {
namespace change_plan {
namespace v1_1 {

const std::string api_path("/api/1.1/billing/change_plan");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            uint32_t product_id,
            InfoOnly info_only
        );

    uint32_t product_id_;
    InfoOnly info_only_;
    boost::optional<std::string> subdomain_;
    boost::optional<std::string> company_;
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
        uint32_t product_id,
        InfoOnly info_only
    ) :
    product_id_(product_id),
    info_only_(info_only)
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

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.nextbilling",
                &optarg) )
        {
            response_data_ptr->next_billing = optarg;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.info_only",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->info_only = InfoOnly::No;
            else if ( optval == "yes" )
                response_data_ptr->info_only = InfoOnly::Yes;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.info_only");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.info_only");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.newpid",
            &response_data_ptr->new_pid ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.newpid\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.amount",
            &response_data_ptr->amount ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.amount\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.interval",
            &response_data_ptr->interval ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.interval\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["product_id"] = AsString(product_id_);
    parts["info_only"] = AsString(info_only_);
    if (subdomain_)
        parts["subdomain"] = *subdomain_;
    if (company_)
        parts["company"] = *company_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        uint32_t product_id,
        InfoOnly info_only
    ) :
    impl_(new Impl(product_id, info_only))
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

void Request::SetSubdomain(std::string subdomain)
{
    impl_->subdomain_ = subdomain;
}

void Request::SetCompany(std::string company)
{
    impl_->company_ = company;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_1
}  // namespace change_plan
}  // namespace billing
}  // namespace api
}  // namespace mf
