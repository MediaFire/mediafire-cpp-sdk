/**
 * @file instant.cpp
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

namespace v1_3 = mf::api::upload::instant::v1_3;


namespace {
std::string AsString(const v1_3::ActionOnDuplicate & value)
{
    if (value == v1_3::ActionOnDuplicate::Skip)
        return "skip";
    if (value == v1_3::ActionOnDuplicate::Keep)
        return "keep";
    if (value == v1_3::ActionOnDuplicate::Replace)
        return "replace";
    return mf::utils::to_string(static_cast<uint32_t>(value));
}
}  // namespace

#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "upload" */
namespace upload {
namespace instant {
namespace v1_3 {

const std::string api_path("/api/1.3/upload/instant");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            std::string filename,
            std::string hash,
            uint64_t filesize
        );

    std::string filename_;
    std::string hash_;
    uint64_t filesize_;
    boost::optional<std::string> quickkey_;
    boost::optional<std::string> target_parent_folderkey_;
    boost::optional<std::string> target_filedrop_;
    boost::optional<std::string> target_path_;
    boost::optional<boost::posix_time::ptime> mtime_;
    boost::optional<ActionOnDuplicate> action_on_duplicate_;
    boost::optional<std::string> previous_hash_;
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
        std::string filename,
        std::string hash,
        uint64_t filesize
    ) :
    filename_(filename),
    hash_(hash),
    filesize_(filesize)
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

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.quickkey",
            &response_data_ptr->quickkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.quickkey\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.filename",
            &response_data_ptr->filename ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.filename\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.new_device_revision",
            &response_data_ptr->new_device_revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.new_device_revision\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["filename"] = filename_;
    parts["hash"] = hash_;
    parts["size"] = AsString(filesize_);
    if (quickkey_)
        parts["quickkey"] = *quickkey_;
    if (target_parent_folderkey_)
        parts["folder_key"] = *target_parent_folderkey_;
    if (target_filedrop_)
        parts["filedrop_key"] = *target_filedrop_;
    if (target_path_)
        parts["path"] = *target_path_;
    if (mtime_)
        parts["mtime"] = AsString(*mtime_);
    if (action_on_duplicate_)
        parts["action_on_duplicate"] = AsString(*action_on_duplicate_);
    if (previous_hash_)
        parts["previous_hash"] = *previous_hash_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string filename,
        std::string hash,
        uint64_t filesize
    ) :
    impl_(new Impl(filename, hash, filesize))
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

void Request::SetQuickkey(std::string quickkey)
{
    impl_->quickkey_ = quickkey;
}

void Request::SetTargetParentFolderkey(std::string target_parent_folderkey)
{
    impl_->target_parent_folderkey_ = target_parent_folderkey;
}

void Request::SetTargetFiledrop(std::string target_filedrop)
{
    impl_->target_filedrop_ = target_filedrop;
}

void Request::SetTargetPath(std::string target_path)
{
    impl_->target_path_ = target_path;
}

void Request::SetMtime(boost::posix_time::ptime mtime)
{
    impl_->mtime_ = mtime;
}

void Request::SetActionOnDuplicate(ActionOnDuplicate action_on_duplicate)
{
    impl_->action_on_duplicate_ = action_on_duplicate;
}

void Request::SetPreviousHash(std::string previous_hash)
{
    impl_->previous_hash_ = previous_hash;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace instant
}  // namespace upload
}  // namespace api
}  // namespace mf
