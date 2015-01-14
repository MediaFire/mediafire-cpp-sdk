/**
 * @file create.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v1_2.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_2 = mf::api::file::create::v1_2;


namespace {
std::string AsString(const v1_2::FileType & value)
{
    if (value == v1_2::FileType::Default)
        return "";
    if (value == v1_2::FileType::Text)
        return "text";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v1_2;  // NOLINT
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
                "one-time.download",
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
                "one-time.listen",
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
                "one-time.watch",
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
                "one-time.view",
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
}  // namespace

namespace mf {
namespace api {
/** API action path "file" */
namespace file {
namespace create {
namespace v1_2 {

const std::string api_path("/api/1.2/file/create");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    boost::optional<std::string> filename_;
    boost::optional<std::string> file_extension_;
    boost::optional<std::string> parent_folderkey_;
    boost::optional<boost::posix_time::ptime> mtime_;
    boost::optional<FileType> file_type_;
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
    response->description = "";
    response->mimetype = "";

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.created",
            &response->created_datetime ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.created\"");

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.fileinfo.description",
            &response->description);

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
            "response.fileinfo.downloads",
            &response->downloads ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.downloads\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.hash",
            &response->filehash ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.hash\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.filename",
            &response->filename ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.filename\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.size",
            &response->filesize ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.size\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.filetype",
            &response->filetype ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.filetype\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.revision",
            &response->file_revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.revision\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.flag",
            &response->flag ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.flag\"");

    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.fileinfo.links");

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
                "missing \"response.fileinfo.links\"");
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // JSON response still has element if no files were returned.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.links\"");
    }

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.fileinfo.mimetype",
            &response->mimetype);

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.fileinfo.password_protected",
                &optval) )
        {
            if ( optval == "no" )
                response->password_protected = PasswordProtected::Unprotected;
            else if ( optval == "yes" )
                response->password_protected = PasswordProtected::Protected;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.fileinfo.password_protected");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.fileinfo.password_protected");
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.fileinfo.privacy",
                &optval) )
        {
            if ( optval == "public" )
                response->privacy = Privacy::Public;
            else if ( optval == "private" )
                response->privacy = Privacy::Private;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.fileinfo.privacy");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.fileinfo.privacy");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.fileinfo.quickkey",
            &response->quickkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.fileinfo.quickkey\"");

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    if (filename_)
        parts["filename"] = *filename_;
    if (file_extension_)
        parts["file_extension"] = *file_extension_;
    if (parent_folderkey_)
        parts["parent_key"] = *parent_folderkey_;
    if (mtime_)
        parts["mtime"] = AsString(*mtime_);
    if (file_type_)
        parts["type"] = AsString(*file_type_);

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

void Request::SetFilename(std::string filename)
{
    impl_->filename_ = filename;
}

void Request::SetFileExtension(std::string file_extension)
{
    impl_->file_extension_ = file_extension;
}

void Request::SetParentFolderkey(std::string parent_folderkey)
{
    impl_->parent_folderkey_ = parent_folderkey;
}

void Request::SetMtime(boost::posix_time::ptime mtime)
{
    impl_->mtime_ = mtime;
}

void Request::SetFileType(FileType file_type)
{
    impl_->file_type_ = file_type;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_2
}  // namespace create
}  // namespace file
}  // namespace api
}  // namespace mf
