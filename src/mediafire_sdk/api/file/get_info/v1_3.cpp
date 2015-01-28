/**
 * @file get_info.cpp
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

namespace v1_3 = mf::api::file::get_info::v1_3;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v1_3;  // NOLINT
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
                "one_time.download",
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
                "one_time.listen",
                &optarg) )
        {
            value->one_time_listen = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "one_time.watch",
                &optarg) )
        {
            value->one_time_watch = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "one_time.view",
                &optarg) )
        {
            value->one_time_view = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "read",
                &optarg) )
        {
            value->read = optarg;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "streaming",
                &optarg) )
        {
            value->streaming = optarg;
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

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
// get_data_type_struct_extractor begin
using namespace v1_3;  // NOLINT
bool PermissionsFromPropertyBranch(
        Response * response,
        Response::Permissions * value,
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
/** API action path "file" */
namespace file {
namespace get_info {
namespace v1_3 {

const std::string api_path("/api/1.3/file/get_info");

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
    response->deleted_datetime = boost::posix_time::not_a_date_time;
    response->mimetype = "";
    response->shared_by_user = SharedByUser::Unshared;

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.quickkey",
            &response->quickkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.quickkey\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.filename",
            &response->filename ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.filename\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.created",
            &response->created_datetime ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.created\"");

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.file_info.delete_date",
            &response->deleted_datetime);

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                response->pt,
                "response.file_info.downloads",
                &optarg) )
        {
            response->download_count = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.size",
            &response->filesize ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.size\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.file_info.privacy",
                &optval) )
        {
            if ( optval == "public" )
                response->privacy = Privacy::Public;
            else if ( optval == "private" )
                response->privacy = Privacy::Private;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.file_info.privacy");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.file_info.privacy");
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.file_info.password_protected",
                &optval) )
        {
            if ( optval == "no" )
                response->password_protected = PasswordProtected::Unprotected;
            else if ( optval == "yes" )
                response->password_protected = PasswordProtected::Protected;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.file_info.password_protected");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.file_info.password_protected");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.hash",
            &response->hash ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.hash\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.filetype",
            &response->filetype ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.filetype\"");

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.file_info.mimetype",
            &response->mimetype);

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.owner_name",
            &response->owner_name ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.owner_name\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.file_info.shared_by_user",
                &optval) )
        {
            if ( optval == "" )
                response->shared_by_user = SharedByUser::Unshared;
            else if ( optval == "1" )
                response->shared_by_user = SharedByUser::Shared;
        }
    }

    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.file_info.permissions");

        Response::Permissions optarg;
        if ( PermissionsFromPropertyBranch(
                response, &optarg, branch) )
        {
            response->permissions = std::move(optarg);
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
                "response.file_info.parent_folderkey",
                &optarg) )
        {
            response->parent_folderkey = optarg;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.revision",
            &response->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.revision\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.file_info.flag",
            &response->flag ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.flag\"");

    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.file_info.links");

        Response::Links optarg;
        if ( LinksFromPropertyBranch(
                response, &optarg, branch) )
        {
            response->links = std::move(optarg);
        }
        else
        {
            // JSON response still has element if expected.
            // This is really an error.
            return_error(
                mf::api::api_code::ContentInvalidData,
                "missing \"response.file_info.links\"");
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // JSON response still has element if no files were returned.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.file_info.links\"");
    }

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
}  // namespace get_info
}  // namespace file
}  // namespace api
}  // namespace mf
