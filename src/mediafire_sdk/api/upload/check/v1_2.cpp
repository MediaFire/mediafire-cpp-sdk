/**
 * @file check.cpp
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

namespace v1_2 = mf::api::upload::check::v1_2;


namespace {
std::string AsString(const v1_2::Resumable & value)
{
    if (value == v1_2::Resumable::NotResumable)
        return "no";
    if (value == v1_2::Resumable::Resumable)
        return "yes";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace {
// get_data_type_struct_extractor begin
using namespace v1_2;  // NOLINT
bool ResumableDataFromPropertyBranch(
        Response * response,
        ResponseData::ResumableData * value,
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
    value->all_units_ready = AllUnitsReady::No;

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "number_of_units",
            &value->number_of_units ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"number_of_units\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                pt,
                "all_units_ready",
                &optval) )
        {
            if ( optval == "no" )
                value->all_units_ready = AllUnitsReady::No;
            else if ( optval == "yes" )
                value->all_units_ready = AllUnitsReady::Yes;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            pt,
            "unit_size",
            &value->unit_size ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"unit_size\"");

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                pt,
                "bitmap.count",
                &optarg) )
        {
            value->bitmap_count = optarg;
        }
    }

    // create_content_parse_array TArray
    try {
        const boost::property_tree::wptree & branch =
            pt.get_child(L"bitmap.words");
        value->words.reserve( branch.size() );
        if (branch.empty())
        {
            return_error(
                mf::api::api_code::ContentInvalidData,
                "missing value in bitmap.words");
        }
        for ( auto & it : branch )
        {
            uint16_t result;
            if ( GetValueIfExists(
                    it.second,
                    &result ) )
            {
                value->words.push_back(result);
            }
            else
            {
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in bitmap.words");
            }
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {
        // JSON response still has element if expected.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"bitmap.words\"");
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                pt,
                "upload_key",
                &optarg) )
        {
            value->upload_key = optarg;
        }
    }

    // get_data_type_struct_extractor conclusion
    return true;
#   undef return_error
}
}  // namespace

namespace mf {
namespace api {
/** API action path "upload" */
namespace upload {
namespace check {
namespace v1_2 {

const std::string api_path("/api/1.2/upload/check");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    explicit Impl(
            std::string filename
        );

    std::string filename_;
    boost::optional<std::string> hash_;
    boost::optional<uint64_t> filesize_;
    boost::optional<std::string> target_parent_folderkey_;
    boost::optional<std::string> target_filedrop_;
    boost::optional<std::string> path_;
    boost::optional<Resumable> resumable_;
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
        std::string filename
    ) :
    filename_(filename)
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
    response_data_ptr->hash_exists = HashAlreadyInSystem::No;
    response_data_ptr->hash_in_account = HashAlreadyInAccount::HashNewToAccount;
    response_data_ptr->hash_in_folder = HashAlreadyInFolder::HashNewToFolder;
    response_data_ptr->file_exists = FilenameInFolder::No;
    response_data_ptr->hash_different = FileExistsWithDifferentHash::No;
    response_data_ptr->available_space = 0;
    response_data_ptr->used_storage_size = 0;
    response_data_ptr->storage_limit = 0;
    response_data_ptr->storage_limit_exceeded = StorageLimitExceeded::No;

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.hash_exists",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->hash_exists = HashAlreadyInSystem::No;
            else if ( optval == "yes" )
                response_data_ptr->hash_exists = HashAlreadyInSystem::Yes;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.in_account",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->hash_in_account = HashAlreadyInAccount::HashNewToAccount;
            else if ( optval == "yes" )
                response_data_ptr->hash_in_account = HashAlreadyInAccount::HashExistsInAccount;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.in_folder",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->hash_in_folder = HashAlreadyInFolder::HashNewToFolder;
            else if ( optval == "yes" )
                response_data_ptr->hash_in_folder = HashAlreadyInFolder::HashExistsInFolder;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.file_exists",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->file_exists = FilenameInFolder::No;
            else if ( optval == "yes" )
                response_data_ptr->file_exists = FilenameInFolder::Yes;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.different_hash",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->hash_different = FileExistsWithDifferentHash::No;
            else if ( optval == "yes" )
                response_data_ptr->hash_different = FileExistsWithDifferentHash::Yes;
        }
    }

    // create_content_parse_single optional no default
    {
        std::string optarg;
        if ( GetIfExists(
                response->pt,
                "response.duplicate_quickkey",
                &optarg) )
        {
            response_data_ptr->duplicate_quickkey = optarg;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.available_space",
            &response_data_ptr->available_space);

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.used_storage_size",
            &response_data_ptr->used_storage_size);

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.storage_limit",
            &response_data_ptr->storage_limit);

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.storage_limit_exceeded",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->storage_limit_exceeded = StorageLimitExceeded::No;
            else if ( optval == "yes" )
                response_data_ptr->storage_limit_exceeded = StorageLimitExceeded::Yes;
        }
    }

    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            response->pt.get_child(L"response.resumable_upload");

        ResponseData::ResumableData optarg;
        if ( ResumableDataFromPropertyBranch(
                response, &optarg, branch) )
        {
            response_data_ptr->resumable = std::move(optarg);
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

    parts["filename"] = filename_;
    if (hash_)
        parts["hash"] = *hash_;
    if (filesize_)
        parts["size"] = AsString(*filesize_);
    if (target_parent_folderkey_)
        parts["folder_key"] = *target_parent_folderkey_;
    if (target_filedrop_)
        parts["filedrop_key"] = *target_filedrop_;
    if (path_)
        parts["path"] = *path_;
    if (resumable_)
        parts["resumable"] = AsString(*resumable_);

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string filename
    ) :
    impl_(new Impl(filename))
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

void Request::SetHash(std::string hash)
{
    impl_->hash_ = hash;
}

void Request::SetFilesize(uint64_t filesize)
{
    impl_->filesize_ = filesize;
}

void Request::SetTargetParentFolderkey(std::string target_parent_folderkey)
{
    impl_->target_parent_folderkey_ = target_parent_folderkey;
}

void Request::SetTargetFiledrop(std::string target_filedrop)
{
    impl_->target_filedrop_ = target_filedrop;
}

void Request::SetPath(std::string path)
{
    impl_->path_ = path;
}

void Request::SetResumable(Resumable resumable)
{
    impl_->resumable_ = resumable;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_2
}  // namespace check
}  // namespace upload
}  // namespace api
}  // namespace mf
