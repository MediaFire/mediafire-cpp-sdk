/**
 * @file get_versions.cpp
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

namespace v1_3 = mf::api::file::get_versions::v1_3;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v1_3;  // NOLINT
bool FileVersionFromPropertyBranch(
        Response * response,
        ResponseData::FileVersion * value,
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
    value->at_head = AtHead::No;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "revision",
            &value->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"revision\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "date",
            &value->date ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"date\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "head",
                &optval) )
        {
            if ( optval == "yes" )
                value->at_head = AtHead::Yes;
            else if ( optval == "no" )
                value->at_head = AtHead::No;
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
namespace get_versions {
namespace v1_3 {

const std::string api_path("/api/1.3/file/get_versions");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    explicit Impl(
            std::string quickkey
        );

    std::string quickkey_;
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
        std::string quickkey
    ) :
    quickkey_(quickkey)
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
            response->pt.get_child(L"response.file_versions");
        response_data_ptr->file_versions.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            ResponseData::FileVersion optarg;
            if ( FileVersionFromPropertyBranch(
                    response, &optarg, it.second) )
                response_data_ptr->file_versions.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["quick_key"] = quickkey_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string quickkey
    ) :
    impl_(new Impl(quickkey))
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

}  // namespace v1_3
}  // namespace get_versions
}  // namespace file
}  // namespace api
}  // namespace mf
