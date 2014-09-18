/**
 * @file cancel_plan.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "cancel_plan.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_1 = mf::api::billing::cancel_plan::v1_1;


namespace {
std::string AsString(const v1_1::CancellationReason & value)
{
    if (value == v1_1::CancellationReason::No)
        return "0";
    if (value == v1_1::CancellationReason::Yes)
        return "1";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "billing" */
namespace billing {
namespace cancel_plan {
namespace v1_1 {

const std::string api_path("/api/1.1/billing/cancel_plan");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    boost::optional<std::string> comments_;
    boost::optional<CancellationReason> uploading_problems_;
    boost::optional<CancellationReason> downloading_problems_;
    boost::optional<CancellationReason> switching_to_other_service_;
    boost::optional<CancellationReason> download_speed_insufficient_;
    boost::optional<CancellationReason> out_of_money_;
    boost::optional<CancellationReason> too_expensive_;
    boost::optional<CancellationReason> no_longer_needed_;
    boost::optional<CancellationReason> lacking_features_;
    boost::optional<CancellationReason> lacking_service_;
    boost::optional<std::string> cc_expiration_;
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

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.cancel_date",
            &response->cancel_date ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.cancel_date\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.confirmation",
            &response->confirmation ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.confirmation\"");

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.last_invoice",
                &optarg) )
        {
            response->last_invoice = optarg;
        }
    }

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    if (comments_)
        parts["comments"] = *comments_;
    if (uploading_problems_)
        parts["x_uploading"] = AsString(*uploading_problems_);
    if (downloading_problems_)
        parts["x_downloading"] = AsString(*downloading_problems_);
    if (switching_to_other_service_)
        parts["x_switching"] = AsString(*switching_to_other_service_);
    if (download_speed_insufficient_)
        parts["x_speed"] = AsString(*download_speed_insufficient_);
    if (out_of_money_)
        parts["x_money_problems"] = AsString(*out_of_money_);
    if (too_expensive_)
        parts["x_too_expensive"] = AsString(*too_expensive_);
    if (no_longer_needed_)
        parts["x_technophobe"] = AsString(*no_longer_needed_);
    if (lacking_features_)
        parts["x_lacking_features"] = AsString(*lacking_features_);
    if (lacking_service_)
        parts["x_lacking_service"] = AsString(*lacking_service_);
    if (cc_expiration_)
        parts["x_other"] = *cc_expiration_;

    std::string post_data = MakePost(api_path + ".php", parts);
    api_data_type_debug_ += " POST data: " + post_data + "\n";
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

void Request::SetComments(std::string comments)
{
    impl_->comments_ = comments;
}

void Request::SetUploadingProblems(CancellationReason uploading_problems)
{
    impl_->uploading_problems_ = uploading_problems;
}

void Request::SetDownloadingProblems(CancellationReason downloading_problems)
{
    impl_->downloading_problems_ = downloading_problems;
}

void Request::SetSwitchingToOtherService(CancellationReason switching_to_other_service)
{
    impl_->switching_to_other_service_ = switching_to_other_service;
}

void Request::SetDownloadSpeedInsufficient(CancellationReason download_speed_insufficient)
{
    impl_->download_speed_insufficient_ = download_speed_insufficient;
}

void Request::SetOutOfMoney(CancellationReason out_of_money)
{
    impl_->out_of_money_ = out_of_money;
}

void Request::SetTooExpensive(CancellationReason too_expensive)
{
    impl_->too_expensive_ = too_expensive;
}

void Request::SetNoLongerNeeded(CancellationReason no_longer_needed)
{
    impl_->no_longer_needed_ = no_longer_needed;
}

void Request::SetLackingFeatures(CancellationReason lacking_features)
{
    impl_->lacking_features_ = lacking_features;
}

void Request::SetLackingService(CancellationReason lacking_service)
{
    impl_->lacking_service_ = lacking_service;
}

void Request::SetCcExpiration(std::string cc_expiration)
{
    impl_->cc_expiration_ = cc_expiration;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_1
}  // namespace cancel_plan
}  // namespace billing
}  // namespace api
}  // namespace mf
