/**
 * @file get_info.cpp
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

namespace v0 = mf::api::folder::get_info::v0;


namespace {
std::string AsString(const v0::Details & value)
{
    if (value == v0::Details::NoDetails)
        return "no";
    if (value == v0::Details::FullDetails)
        return "yes";
    if (value == v0::Details::ShallowDetails)
        return "shallow";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v0;  // NOLINT
bool PermissionsFromPropertyBranch(
        Response * response,
        ResponseData::Permissions * value,
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
            "value",
            &value->value ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"value\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "explicit",
                &optval) )
        {
            if ( optval == "0" )
                value->explicit_share = ExplicitShare::No;
            else if ( optval == "1" )
                value->explicit_share = ExplicitShare::Yes;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in explicit");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in explicit");
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "read",
                &optval) )
        {
            if ( optval == "1" )
                value->read_permission = Permission::Allowed;
            else if ( optval == "0" )
                value->read_permission = Permission::Denied;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in read");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in read");
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "write",
                &optval) )
        {
            if ( optval == "1" )
                value->write_permission = Permission::Allowed;
            else if ( optval == "0" )
                value->write_permission = Permission::Denied;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in write");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in write");
    }

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "folder" */
namespace folder {
namespace get_info {
namespace v0 {

const std::string api_path("/api/folder/get_info");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    explicit Impl(
            std::string folderkey
        );

    std::string folderkey_;
    boost::optional<Details> details_;
    boost::optional<uint32_t> device_id_;
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
        std::string folderkey
    ) :
    folderkey_(folderkey)
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
    response_data_ptr->created_datetime = boost::posix_time::not_a_date_time;
    response_data_ptr->deleted_datetime = boost::posix_time::not_a_date_time;
    response_data_ptr->shared_by_user = SharedByUser::Unshared;

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_info.folderkey",
            &response_data_ptr->folderkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_info.folderkey\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_info.name",
            &response_data_ptr->name ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_info.name\"");

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.description",
                &optarg) )
        {
            response_data_ptr->description = optarg;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.folder_info.created",
            &response_data_ptr->created_datetime);

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.folder_info.delete_date",
            &response_data_ptr->deleted_datetime);

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.parent_folderkey",
                &optarg) )
        {
            response_data_ptr->parent_folderkey = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.custom_url",
                &optarg) )
        {
            response_data_ptr->custom_url = optarg;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.folder_info.dropbox_enabled",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->filedrop = FileDrop::Disabled;
            else if ( optval == "yes" )
                response_data_ptr->filedrop = FileDrop::Enabled;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_info.file_count",
            &response_data_ptr->file_count ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_info.file_count\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_info.folder_count",
            &response_data_ptr->folder_count ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_info.folder_count\"");

    // create_content_parse_single optional no default
    {
        uint64_t optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.size",
                &optarg) )
        {
            response_data_ptr->size = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.total_folders",
                &optarg) )
        {
            response_data_ptr->total_folders = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.total_files",
                &optarg) )
        {
            response_data_ptr->total_files = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        uint64_t optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.total_size",
                &optarg) )
        {
            response_data_ptr->total_size = optarg;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.folder_info.shared_by_user",
                &optval) )
        {
            if ( optval == "" )
                response_data_ptr->shared_by_user = SharedByUser::Unshared;
            else if ( optval == "1" )
                response_data_ptr->shared_by_user = SharedByUser::Shared;
        }
    }

    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"permissions");

        ResponseData::Permissions optarg;
        if ( PermissionsFromPropertyBranch(
                response, &optarg, branch) )
        {
            response_data_ptr->permissions = std::move(optarg);
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.owner_name",
                &optarg) )
        {
            response_data_ptr->owner_name = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.folder_info.avatar",
                &optarg) )
        {
            response_data_ptr->avatar = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_info.flag",
            &response_data_ptr->flag ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_info.flag\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_info.revision",
            &response_data_ptr->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_info.revision\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["folder_key"] = folderkey_;
    if (details_)
        parts["details"] = AsString(*details_);
    if (device_id_)
        parts["device_id"] = AsString(*device_id_);

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string folderkey
    ) :
    impl_(new Impl(folderkey))
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

void Request::SetDetails(Details details)
{
    impl_->details_ = details;
}

void Request::SetDeviceId(uint32_t device_id)
{
    impl_->device_id_ = device_id;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v0
}  // namespace get_info
}  // namespace folder
}  // namespace api
}  // namespace mf
