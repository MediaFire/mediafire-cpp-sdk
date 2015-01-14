/**
 * @file get_content.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v1_0.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_0 = mf::api::folder::get_content::v1_0;


namespace {
std::string AsString(const v1_0::ContentType & value)
{
    if (value == v1_0::ContentType::Folders)
        return "folders";
    if (value == v1_0::ContentType::Files)
        return "files";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_0::OrderBy & value)
{
    if (value == v1_0::OrderBy::Name)
        return "name";
    if (value == v1_0::OrderBy::Created)
        return "created";
    if (value == v1_0::OrderBy::Size)
        return "size";
    if (value == v1_0::OrderBy::Downloads)
        return "downloads";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_0::OrderDirection & value)
{
    if (value == v1_0::OrderDirection::Ascending)
        return "asc";
    if (value == v1_0::OrderDirection::Descending)
        return "desc";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
std::string AsString(const v1_0::Details & value)
{
    if (value == v1_0::Details::NoDetails)
        return "no";
    if (value == v1_0::Details::PopulateDetails)
        return "yes";
    if (value == v1_0::Details::ShallowDetails)
        return "shallow";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v1_0;  // NOLINT
bool LinksFromPropertyBranch(
        Response * response,
        Response::Links * value,
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
                "view",
                &optarg) )
        {
            value->view = optarg;
        }
    }

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
// get_data_type_struct_extractor begin
using namespace v1_0;  // NOLINT
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
    using mf::api::GetValueIfExists;
    value->deleted_datetime = boost::posix_time::not_a_date_time;
    value->mimetype = "";
    value->shared_by_user = SharedByUser::Unshared;
    value->shared_with_user = SharedWithUser::UserIsOwner;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "created",
            &value->created_datetime ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"created\"");

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "delete_date",
            &value->deleted_datetime);

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "description",
                &optarg) )
        {
            value->description = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "downloads",
            &value->download_count ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"downloads\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "filename",
            &value->filename ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"filename\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "size",
            &value->filesize ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"size\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "filetype",
            &value->filetype ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"filetype\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "flag",
            &value->flag ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"flag\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "hash",
            &value->hash ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"hash\"");

    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            pt.get_child(L"links");

        Response::Links optarg;
        if ( LinksFromPropertyBranch(
                response, &optarg, branch) )
        {
            value->links = std::move(optarg);
        }
        else
        {
            // JSON response still has element if expected.
            // This is really an error.
            return_error(
                mf::api::api_code::ContentInvalidData,
                "missing \"links\"");
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // JSON response still has element if no files were returned.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"links\"");
    }

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "mimetype",
            &value->mimetype);

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "password_protected",
                &optval) )
        {
            if ( optval == "no" )
                value->password_protected = PasswordProtected::Unprotected;
            else if ( optval == "yes" )
                value->password_protected = PasswordProtected::Protected;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in password_protected");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in password_protected");
    }

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                pt,
                "permissions.value",
                &optarg) )
        {
            value->permissions = optarg;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "privacy",
                &optval) )
        {
            if ( optval == "public" )
                value->privacy = Privacy::Public;
            else if ( optval == "private" )
                value->privacy = Privacy::Private;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in privacy");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in privacy");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "quickkey",
            &value->quickkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"quickkey\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "revision",
            &value->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"revision\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "shared_by_user",
                &optval) )
        {
            if ( optval == "" )
                value->shared_by_user = SharedByUser::Unshared;
            else if ( optval == "1" )
                value->shared_by_user = SharedByUser::Shared;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "shared",
                &optval) )
        {
            if ( optval == "no" )
                value->shared_with_user = SharedWithUser::UserIsOwner;
            else if ( optval == "yes" )
                value->shared_with_user = SharedWithUser::SharedWithUser;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "views",
            &value->views ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"views\"");

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
// get_data_type_struct_extractor begin
using namespace v1_0;  // NOLINT
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
    using mf::api::GetValueIfExists;
    value->deleted_datetime = boost::posix_time::not_a_date_time;
    value->shared_by_user = SharedByUser::Unshared;
    value->shared_with_user = SharedWithUser::UserIsOwner;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "created",
            &value->created_datetime ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"created\"");

    // create_content_parse_single optional with default
    GetIfExists(
            pt,
            "delete_date",
            &value->deleted_datetime);

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "description",
                &optarg) )
        {
            value->description = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "file_count",
            &value->file_count ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"file_count\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "dropbox_enabled",
                &optval) )
        {
            if ( optval == "no" )
                value->filedrop = FileDrop::Disabled;
            else if ( optval == "yes" )
                value->filedrop = FileDrop::Enabled;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in dropbox_enabled");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in dropbox_enabled");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "flag",
            &value->flag ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"flag\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "folder_count",
            &value->folder_count ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"folder_count\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "folderkey",
            &value->folderkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"folderkey\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "name",
            &value->name ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"name\"");

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                pt,
                "permissions.value",
                &optarg) )
        {
            value->permissions = optarg;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "privacy",
                &optval) )
        {
            if ( optval == "public" )
                value->privacy = Privacy::Public;
            else if ( optval == "private" )
                value->privacy = Privacy::Private;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in privacy");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in privacy");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "revision",
            &value->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"revision\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "shared_by_user",
                &optval) )
        {
            if ( optval == "" )
                value->shared_by_user = SharedByUser::Unshared;
            else if ( optval == "1" )
                value->shared_by_user = SharedByUser::Shared;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "shared",
                &optval) )
        {
            if ( optval == "no" )
                value->shared_with_user = SharedWithUser::UserIsOwner;
            else if ( optval == "yes" )
                value->shared_with_user = SharedWithUser::SharedWithUser;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "tags",
                &optarg) )
        {
            value->tags = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                pt,
                "total_files",
                &optarg) )
        {
            value->total_files = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                pt,
                "total_folders",
                &optarg) )
        {
            value->total_folders = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        uint64_t optarg;
        if ( GetIfExists(
                pt,
                "total_size",
                &optarg) )
        {
            value->total_size = optarg;
        }
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
namespace get_content {
namespace v1_0 {

const std::string api_path("/api/1.0/folder/get_content");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            std::string folder_key,
            uint32_t chunk_number,
            ContentType content_type
        );

    std::string folder_key_;
    uint32_t chunk_number_;
    ContentType content_type_;
    boost::optional<uint32_t> device_id_;
    boost::optional<OrderBy> order_by_;
    boost::optional<OrderDirection> order_direction_;
    boost::optional<std::string> filter_;
    boost::optional<Details> details_;
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
        std::string folder_key,
        uint32_t chunk_number,
        ContentType content_type
    ) :
    folder_key_(folder_key),
    chunk_number_(chunk_number),
    content_type_(content_type)
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
            "response.folder_content.chunk_size",
            &response->chunk_size ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_content.chunk_size\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.folder_content.content_type",
                &optval) )
        {
            if ( optval == "folders" )
                response->content_type = ContentType::Folders;
            else if ( optval == "files" )
                response->content_type = ContentType::Files;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.folder_content.content_type");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.folder_content.content_type");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.folder_content.chunk_number",
            &response->chunk_number ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.folder_content.chunk_number\"");

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.folder_content.files");
        response->files.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            Response::File optarg;
            if ( FileFromPropertyBranch(
                    response, &optarg, it.second) )
                response->files.push_back(std::move(optarg));
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // Is optional
    }

    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.folder_content.folders");
        response->folders.reserve( response->pt.size() );

        for ( auto & it : branch )
        {
            Response::Folder optarg;
            if ( FolderFromPropertyBranch(
                    response, &optarg, it.second) )
                response->folders.push_back(std::move(optarg));
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

    parts["folder_key"] = folder_key_;
    parts["chunk"] = AsString(chunk_number_);
    parts["content_type"] = AsString(content_type_);
    if (device_id_)
        parts["device_id"] = AsString(*device_id_);
    if (order_by_)
        parts["order_by"] = AsString(*order_by_);
    if (order_direction_)
        parts["order_direction"] = AsString(*order_direction_);
    if (filter_)
        parts["filter"] = *filter_;
    if (details_)
        parts["details"] = AsString(*details_);

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string folder_key,
        uint32_t chunk_number,
        ContentType content_type
    ) :
    impl_(new Impl(folder_key, chunk_number, content_type))
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

void Request::SetDeviceId(uint32_t device_id)
{
    impl_->device_id_ = device_id;
}

void Request::SetOrderBy(OrderBy order_by)
{
    impl_->order_by_ = order_by;
}

void Request::SetOrderDirection(OrderDirection order_direction)
{
    impl_->order_direction_ = order_direction;
}

void Request::SetFilter(std::string filter)
{
    impl_->filter_ = filter;
}

void Request::SetDetails(Details details)
{
    impl_->details_ = details;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_0
}  // namespace get_content
}  // namespace folder
}  // namespace api
}  // namespace mf
