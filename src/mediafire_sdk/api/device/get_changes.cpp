/**
 * @file get_changes.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "get_changes.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::device::get_changes::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v0;  // NOLINT
bool FileFromPropertyBranch(
        Response * response,
        Response::File * value,
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
    value->created_datetime = boost::posix_time::not_a_date_time;

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
                "parent_folderkey",
                &optarg) )
        {
            value->parent_folderkey = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "revision",
            &value->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"revision\"");

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "created",
            &value->created_datetime);

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
// get_data_type_struct_extractor begin
using namespace v0;  // NOLINT
bool FolderFromPropertyBranch(
        Response * response,
        Response::Folder * value,
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
    value->created_datetime = boost::posix_time::not_a_date_time;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "folderkey",
            &value->folderkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"folderkey\"");

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "parent_folderkey",
                &optarg) )
        {
            value->parent_folderkey = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "revision",
            &value->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"revision\"");

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "created",
            &value->created_datetime);

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "device" */
namespace device {
namespace get_changes {
namespace v0 {

const std::string api_path("/api/device/get_changes");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    explicit Impl(
            uint32_t revision
        );

    uint32_t revision_;
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
        uint32_t revision
    ) :
    revision_(revision)
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
            "response.device_revision",
            &response->device_revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.device_revision\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.changes_list_block",
            &response->changes_list_block ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.changes_list_block\"");

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.updated.files");
        response->updated_files.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            Response::File optarg;
            if ( FileFromPropertyBranch(
                    response, &optarg, it.second) )
                response->updated_files.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.deleted.files");
        response->deleted_files.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            Response::File optarg;
            if ( FileFromPropertyBranch(
                    response, &optarg, it.second) )
                response->deleted_files.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.updated.folders");
        response->updated_folders.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            Response::Folder optarg;
            if ( FolderFromPropertyBranch(
                    response, &optarg, it.second) )
                response->updated_folders.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.deleted.folders");
        response->deleted_folders.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            Response::Folder optarg;
            if ( FolderFromPropertyBranch(
                    response, &optarg, it.second) )
                response->deleted_folders.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["revision"] = AsString(revision_);

    std::string post_data = MakePost(api_path + ".php", parts);
    api_data_type_debug_ += " POST data: " + post_data + "\n";
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        uint32_t revision
    ) :
    impl_(new Impl(revision))
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
}  // namespace get_changes
}  // namespace device
}  // namespace api
}  // namespace mf
