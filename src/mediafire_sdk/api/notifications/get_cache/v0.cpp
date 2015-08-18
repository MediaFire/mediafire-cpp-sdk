/**
 * @file get_cache.cpp
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

namespace v0 = mf::api::notifications::get_cache::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v0;  // NOLINT
bool NotificationFromPropertyBranch(
        Response * response,
        ResponseData::Notification * value,
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
    value->timestamp = boost::posix_time::not_a_date_time;
    value->viewable = Viewable::Hidden;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "actor",
            &value->actor ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"actor\"");

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "timestamp",
            &value->timestamp);

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "message",
            &value->message ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"message\"");

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "resource",
                &optarg) )
        {
            value->resource = optarg;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "viewable",
                &optval) )
        {
            if ( optval == "0" )
                value->viewable = Viewable::Hidden;
            else if ( optval == "1" )
                value->viewable = Viewable::Viewable;
        }
    }

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "notifications" */
namespace notifications {
namespace get_cache {
namespace v0 {

const std::string api_path("/api/notifications/get_cache");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    boost::optional<uint32_t> start_;
    boost::optional<uint32_t> limit_;
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

    ResponseData response_data;

    // For uniformity for code generation with the other content parsers.
    ResponseData * response_data_ptr = &response_data;

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.notifications");
        response_data_ptr->notifications.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            ResponseData::Notification optarg;
            if ( NotificationFromPropertyBranch(
                    response, &optarg, it.second) )
                response_data_ptr->notifications.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.num_older",
            &response_data_ptr->number_older ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.num_older\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    if (start_)
        parts["start"] = AsString(*start_);
    if (limit_)
        parts["limit"] = AsString(*limit_);

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

void Request::SetStart(uint32_t start)
{
    impl_->start_ = start;
}

void Request::SetLimit(uint32_t limit)
{
    impl_->limit_ = limit;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v0
}  // namespace get_cache
}  // namespace notifications
}  // namespace api
}  // namespace mf
