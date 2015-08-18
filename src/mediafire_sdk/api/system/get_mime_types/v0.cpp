/**
 * @file get_mime_types.cpp
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
#include "mediafire_sdk/api/tokenless_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::system::get_mime_types::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v0;  // NOLINT
bool MimeTypeFromPropertyBranch(
        Response * response,
        ResponseData::MimeType * value,
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

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "mimetype",
            &value->mimetype ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"mimetype\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "extension",
            &value->extension ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"extension\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "type",
                &optval) )
        {
            if ( optval == "unknown" )
                value->category = Category::Unknown;
            else if ( optval == "image" )
                value->category = Category::Image;
            else if ( optval == "video" )
                value->category = Category::Video;
            else if ( optval == "audio" )
                value->category = Category::Audio;
            else if ( optval == "document" )
                value->category = Category::Document;
            else if ( optval == "application" )
                value->category = Category::Application;
            else if ( optval == "archive" )
                value->category = Category::Archive;
            else if ( optval == "data" )
                value->category = Category::Data;
            else if ( optval == "development" )
                value->category = Category::Development;
            else if ( optval == "spreadsheet" )
                value->category = Category::Spreadsheet;
            else if ( optval == "presentation" )
                value->category = Category::Presentation;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "view",
                &optval) )
        {
            if ( optval == "0" )
                value->view_system = ViewSystem::None;
            else if ( optval == "1" )
                value->view_system = ViewSystem::Pdf2html;
            else if ( optval == "2" )
                value->view_system = ViewSystem::Pdf_js;
            else if ( optval == "3" )
                value->view_system = ViewSystem::Zoho;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "edit",
                &optval) )
        {
            if ( optval == "0" )
                value->edit_system = EditSystem::None;
            else if ( optval == "1" )
                value->edit_system = EditSystem::Ace;
            else if ( optval == "2" )
                value->edit_system = EditSystem::Zoho;
        }
    }

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "system" */
namespace system {
namespace get_mime_types {
namespace v0 {

const std::string api_path("/api/system/get_mime_types");

// Impl ------------------------------------------------------------------------

class Impl : public TokenlessApiBase<Response>
{
public:
    Impl();

    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * response ) override;

    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Get;
    }
};

Impl::Impl()
{
}

void Impl::BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * /* query_parts */
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
            response->pt.get_child(L"response.mime_types");
        response_data_ptr->mimetypes.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            ResponseData::MimeType optarg;
            if ( MimeTypeFromPropertyBranch(
                    response, &optarg, it.second) )
                response_data_ptr->mimetypes.push_back(std::move(optarg));
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
            "missing \"response.mime_types\"");
    }

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data); 

#   undef return_error
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

}  // namespace v0
}  // namespace get_mime_types
}  // namespace system
}  // namespace api
}  // namespace mf
