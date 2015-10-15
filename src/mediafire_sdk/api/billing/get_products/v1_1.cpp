/**
 * @file get_products.cpp
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
#include "mediafire_sdk/api/tokenless_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_1 = mf::api::billing::get_products::v1_1;


namespace {
std::string AsString(const v1_1::Recurring & value)
{
    if (value == v1_1::Recurring::NonRecurring)
        return "0";
    if (value == v1_1::Recurring::Recurring)
        return "1";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_1::Activity & value)
{
    if (value == v1_1::Activity::Inactive)
        return "0";
    if (value == v1_1::Activity::Active)
        return "1";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_1::Legacy & value)
{
    if (value == v1_1::Legacy::No)
        return "0";
    if (value == v1_1::Legacy::Yes)
        return "1";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v1_1;  // NOLINT
bool ProductFromPropertyBranch(
        Response * response,
        ResponseData::Product * value,
        const boost::property_tree::wptree & pt
    )
{
#   define return_error(error_type, error_message)                             \
    {                                                                          \
        response->error_code = make_error_code( error_type );                  \
        response->error_string = error_message;                                \
        return false;                                                          \
    }
    using mf::api::GetIfExists;
    using mf::api::GetValueIfExists;
    if (pt.size() == 0)  // Stop if branch is empty
        return false;
    value->is_recurring = Recurring::NonRecurring;
    value->recurring_units = 0;
    value->trial = Trial::NoTrial;
    value->active = Activity::Inactive;
    value->frequency_text = "";
    value->uses_credits = UsesCredits::NoCredits;
    value->reseller_entitlement = -1;
    value->yearly_only = YearlyOnly::No;
    value->legacy = Legacy::No;
    value->product_class = 0;
    value->product_family = 0;
    value->base_storage = 0;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "product_id",
            &value->product_id ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"product_id\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "description",
            &value->description ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"description\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "short_description",
            &value->short_description ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"short_description\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "initial_amount",
            &value->initial_amount ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"initial_amount\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "initial_units",
            &value->initial_units ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"initial_units\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "is_recurring",
                &optval) )
        {
            if ( optval == "0" )
                value->is_recurring = Recurring::NonRecurring;
            else if ( optval == "1" )
                value->is_recurring = Recurring::Recurring;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "recurring_amount",
            &value->recurring_amount ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"recurring_amount\"");

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "recurring_units",
            &value->recurring_units);

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "free_months",
            &value->free_months ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"free_months\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "trial",
                &optval) )
        {
            if ( optval == "0" )
                value->trial = Trial::NoTrial;
            else if ( optval == "1" )
                value->trial = Trial::Trial;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "active",
                &optval) )
        {
            if ( optval == "0" )
                value->active = Activity::Inactive;
            else if ( optval == "1" )
                value->active = Activity::Active;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "frequency_text",
            &value->frequency_text);

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "uses_credits",
                &optval) )
        {
            if ( optval == "0" )
                value->uses_credits = UsesCredits::NoCredits;
            else if ( optval == "1" )
                value->uses_credits = UsesCredits::Credits;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "reseller_entitlement",
            &value->reseller_entitlement);

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "payment_methods",
            &value->payment_methods ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"payment_methods\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "yearly_only",
                &optval) )
        {
            if ( optval == "0" )
                value->yearly_only = YearlyOnly::No;
            else if ( optval == "1" )
                value->yearly_only = YearlyOnly::Yes;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "interval",
            &value->interval ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"interval\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "legacy",
                &optval) )
        {
            if ( optval == "0" )
                value->legacy = Legacy::No;
            else if ( optval == "1" )
                value->legacy = Legacy::Yes;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "product_class",
            &value->product_class);

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "product_family",
            &value->product_family);

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "base_storage",
            &value->base_storage);

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "billing" */
namespace billing {
namespace get_products {
namespace v1_1 {

const std::string api_path("/api/1.1/billing/get_products");

// Impl ------------------------------------------------------------------------

class Impl : public TokenlessApiBase<Response>
{
public:
    Impl();

    boost::optional<Activity> active_;
    boost::optional<Legacy> legacy_;
    boost::optional<uint32_t> interval_;
    boost::optional<Recurring> recurring_;
    boost::optional<uint32_t> product_class_;
    boost::optional<uint32_t> product_family_;
    boost::optional<uint32_t> product_id_;
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

    ResponseData response_data;

    // For uniformity for code generation with the other content parsers.
    ResponseData * response_data_ptr = &response_data;

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.products");
        response_data_ptr->products.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            ResponseData::Product optarg;
            if ( ProductFromPropertyBranch(
                    response, &optarg, it.second) )
                response_data_ptr->products.push_back(std::move(optarg));
            else
                return;  // error set already
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // JSON response still has element if no files were returned.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.products\"");
    }

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    if (active_)
        parts["active"] = AsString(*active_);
    if (legacy_)
        parts["legacy"] = AsString(*legacy_);
    if (interval_)
        parts["interval"] = AsString(*interval_);
    if (recurring_)
        parts["recurring"] = AsString(*recurring_);
    if (product_class_)
        parts["product_class"] = AsString(*product_class_);
    if (product_family_)
        parts["product_family"] = AsString(*product_family_);
    if (product_id_)
        parts["product_id"] = AsString(*product_id_);

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

void Request::SetActive(Activity active)
{
    impl_->active_ = active;
}

void Request::SetLegacy(Legacy legacy)
{
    impl_->legacy_ = legacy;
}

void Request::SetInterval(uint32_t interval)
{
    impl_->interval_ = interval;
}

void Request::SetRecurring(Recurring recurring)
{
    impl_->recurring_ = recurring;
}

void Request::SetProductClass(uint32_t product_class)
{
    impl_->product_class_ = product_class;
}

void Request::SetProductFamily(uint32_t product_family)
{
    impl_->product_family_ = product_family;
}

void Request::SetProductId(uint32_t product_id)
{
    impl_->product_id_ = product_id;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_1
}  // namespace get_products
}  // namespace billing
}  // namespace api
}  // namespace mf
