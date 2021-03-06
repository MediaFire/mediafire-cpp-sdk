/**
 * @file api/file/get_info.hpp
 * @brief API request: /api/file/get_info
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
#pragma once

#include <string>
#include <vector>

#include "mediafire_sdk/http/shared_buffer.hpp"
#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace api {
/** API action path "file" */
namespace file {
/** API action "file/get_info" */
namespace get_info {
/** API path "/api/file/get_info" */
namespace v0 {

enum class Privacy
{
    /** API value "public" */
    Public,
    /** API value "private" */
    Private
};

enum class PasswordProtected
{
    /** API value "no" */
    Unprotected,
    /** API value "yes" */
    Protected
};

enum class SharedByUser
{
    /** API value "" */
    Unshared,
    /** API value "1" */
    Shared
};

enum class ExplicitShare
{
    /** API value "0" */
    No,
    /** API value "1" */
    Yes
};

enum class Permission
{
    /** API value "1" */
    Allowed,
    /** API value "0" */
    Denied
};

/**
 * @class ResponseData
 * @brief Response data from API request "file/get_info"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        deleted_datetime(boost::posix_time::not_a_date_time),
        mimetype(""),
        shared_by_user(SharedByUser::Unshared)
    {}
    struct Permissions
    {
        /** API response field "value" */
        uint32_t value;

        /** API response field "explicit" */
        ExplicitShare explicit_share;

        /** API response field "read" */
        Permission read_permission;

        /** API response field "write" */
        Permission write_permission;
    };
    /** API response field "response.file_info.quickkey" */
    std::string quickkey;

    /** API response field "response.file_info.filename" */
    std::string filename;

    /** API response field "response.file_info.created" */
    boost::posix_time::ptime created_datetime;

    /** API response field "response.file_info.delete_date" */
    boost::posix_time::ptime deleted_datetime;

    /** API response field "response.file_info.downloads" */
    boost::optional<uint32_t> download_count;

    /** API response field "response.file_info.size" */
    uint64_t filesize;

    /** API response field "response.file_info.privacy" */
    Privacy privacy;

    /** API response field "response.file_info.password_protected" */
    PasswordProtected password_protected;

    /** API response field "response.file_info.hash" */
    std::string hash;

    /** API response field "response.file_info.filetype" */
    std::string filetype;

    /** API response field "response.file_info.mimetype" */
    std::string mimetype;

    /** API response field "response.file_info.owner_name" */
    std::string owner_name;

    /** API response field "response.file_info.shared_by_user" */
    SharedByUser shared_by_user;

    /** API response field "response.file_info.permissions" */
    boost::optional<Permissions> permissions;

    /** API response field "response.file_info.parent_folderkey" */
    boost::optional<std::string> parent_folderkey;

    /** API response field "response.file_info.revision" */
    uint32_t revision;

    /** API response field "response.file_info.flag" */
    uint32_t flag;
};

/**
 * @class Response
 * @brief Response from API request "file/get_info"
 */
class Response : public ResponseBase
{
public:
    /** Parsed API response on successful parse. */
    boost::optional<ResponseData> response_data;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "file/get_info"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Privacy = ::mf::api::file::get_info::v0::Privacy;
    using PasswordProtected = ::mf::api::file::get_info::v0::PasswordProtected;
    using SharedByUser = ::mf::api::file::get_info::v0::SharedByUser;
    using ExplicitShare = ::mf::api::file::get_info::v0::ExplicitShare;
    using Permission = ::mf::api::file::get_info::v0::Permission;

    /**
     * API request "file/get_info"
     *
     * @param quickkey API parameter "quick_key"
     */
    explicit Request(
            std::string quickkey
        );

    // Remaining functions are for use by API library only. --------------------

    /** Requester/SessionMaintainer expected type. */
    using ResponseType = Response;

    /** Requester/SessionMaintainer expected type. */
    using ResponseDataType = ResponseData;

    /** Requester/SessionMaintainer expected type. */
    typedef std::function< void( const ResponseType & data)> CallbackType;

    /** Requester/SessionMaintainer expected type. */
    void SetCallback( CallbackType callback_function );

    /** Requester expected method. */
    void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content
        );

    /** Requester expected method. */
    void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string
        );

    /** Requester expected method. */
    std::string Url(const std::string & hostname) const;

    /** Requester optional method. */
    mf::http::SharedBuffer::Pointer GetPostData();

    /** SessionMaintainer expected method. */
    void SetSessionToken(
            std::string session_token,
            std::string time,
            int secret_key
        );
private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v0

}  // namespace get_info
}  // namespace file
}  // namespace api
}  // namespace mf
