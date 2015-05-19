/**
 * @file api/folder/create.hpp
 * @brief API request: /api/folder/create
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
/** API action path "folder" */
namespace folder {
/** API action "folder/create" */
namespace create {
/** API path "/api/folder/create" */
namespace v0 {

enum class OnNameCollision
{
    /** API value "yes" */
    Rename,
    /** API value "no" */
    ReturnExistingFolderkey
};

enum class FileDrop
{
    /** API value "no" */
    Disabled,
    /** API value "yes" */
    Enabled
};

enum class Privacy
{
    /** API value "public" */
    Public,
    /** API value "private" */
    Private
};

/**
 * @class Response
 * @brief Response from API request "folder/create"
 */
class Response : public ResponseBase
{
public:
    /** API response field "response.folder_key" */
    std::string folderkey;

    /** API response field "response.upload_key" */
    std::string uploadkey;

    /** API response field "response.name" */
    std::string name;

    /** API response field "response.created" */
    boost::posix_time::ptime created_datetime;

    /** API response field "response.privacy" */
    Privacy privacy;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "folder/create"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using OnNameCollision = enum OnNameCollision;
    using FileDrop = enum FileDrop;
    using Privacy = enum Privacy;

    /**
     * API request "folder/create"
     *
     * @param foldername API parameter "foldername"
     */
    explicit Request(
            std::string foldername
        );

    /**
     * Optional API parameter "parent_key"
     *
     * @param parent_folderkey Set parameter "parent_key" in API request.
     */
    void SetParentFolderkey(std::string parent_folderkey);

    /**
     * Optional API parameter "mtime"
     *
     * @param mtime Set parameter "mtime" in API request.
     */
    void SetMtime(boost::posix_time::ptime mtime);

    /**
     * Optional API parameter "allow_duplicate_name"
     *
     * @param rename_on_name_collision Set parameter "allow_duplicate_name" in
     *                                 API request.
     */
    void SetRenameOnNameCollision(OnNameCollision rename_on_name_collision);

    // Remaining functions are for use by API library only. --------------------

    /** Requester/SessionMaintainer expected type. */
    typedef Response ResponseType;

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

}  // namespace create
}  // namespace folder
}  // namespace api
}  // namespace mf
