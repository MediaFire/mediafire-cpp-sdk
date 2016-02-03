/**
 * @file get_links.cpp
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

namespace v0 = mf::api::file::get_links::v0;


namespace {
std::string AsString(const v0::LinkType & value)
{
    if (value == v0::LinkType::View)
        return "view";
    if (value == v0::LinkType::Edit)
        return "edit";
    if (value == v0::LinkType::Listen)
        return "listen";
    if (value == v0::LinkType::Watch)
        return "watch";
    if (value == v0::LinkType::NormalDownload)
        return "normal_download";
    if (value == v0::LinkType::DirectDownload)
        return "direct_download";
    if (value == v0::LinkType::OneTimeDownload)
        return "one_time_download";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v0;  // NOLINT
bool LinksFromPropertyBranch(
        Response * response,
        ResponseData::Links * value,
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

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "direct_download",
                &optarg) )
        {
            value->direct_download = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "direct_download_error_message",
                &optarg) )
        {
            value->direct_download_error_message = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        int32_t optarg;
        if ( GetIfExists(
                pt,
                "direct_download_error",
                &optarg) )
        {
            value->direct_download_error = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "edit",
                &optarg) )
        {
            value->edit = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "edit_error_message",
                &optarg) )
        {
            value->edit_error_message = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "listen",
                &optarg) )
        {
            value->listen = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "normal_download",
                &optarg) )
        {
            value->normal_download = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "one_time_download",
                &optarg) )
        {
            value->one_time_download = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "one_time_download_error_message",
                &optarg) )
        {
            value->one_time_download_error_message = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "quickkey",
            &value->quickkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"quickkey\"");

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "view",
                &optarg) )
        {
            value->view = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "watch",
                &optarg) )
        {
            value->watch = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "error",
                &optarg) )
        {
            value->error = optarg;
        }
    }

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "file" */
namespace file {
namespace get_links {
namespace v0 {

const std::string api_path("/api/file/get_links");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    explicit Impl(
            std::vector<std::string> quickkeys
        );

    std::vector<std::string> quickkeys_;
    boost::optional<std::set<LinkType>> link_types_;
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
        std::vector<std::string> quickkeys
    ) :
    quickkeys_(quickkeys)
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
    response_data_ptr->one_time_download_request_count = 0;
    response_data_ptr->direct_download_free_bandwidth = 0;

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.links");
        response_data_ptr->links.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            ResponseData::Links optarg;
            if ( LinksFromPropertyBranch(
                    response, &optarg, it.second) )
                response_data_ptr->links.push_back(std::move(optarg));
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
            "missing \"response.links\"");
    }

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.one_time_download_request_count",
            &response_data_ptr->one_time_download_request_count);

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.direct_download_free_bandwidth",
            &response_data_ptr->direct_download_free_bandwidth);

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["quick_key"] = AsString(quickkeys_);
    if (link_types_)
        parts["link_type"] = AsString(*link_types_);

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::vector<std::string> quickkeys
    ) :
    impl_(new Impl(quickkeys))
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

void Request::SetLinkTypes(std::set<LinkType> link_types)
{
    impl_->link_types_ = link_types;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v0
}  // namespace get_links
}  // namespace file
}  // namespace api
}  // namespace mf
