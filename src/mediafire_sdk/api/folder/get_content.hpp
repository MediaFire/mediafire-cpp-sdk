/**
 * @file api/folder/get_content.hpp
 * @brief API request: /api/1.0/folder/get_content
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
/** API action "folder/get_content" */
namespace get_content {
/** API path "/api/1.0/folder/get_content" */
namespace v2_0 {

enum class ContentType
{
    /** API value "folders" */
    Folders,
    /** API value "files" */
    Files
};

enum class OrderBy
{
    /** API value "name" */
    Name,
    /** API value "created" */
    Created,
    /** API value "size" */
    Size,
    /** API value "downloads" */
    Downloads
};

enum class OrderDirection
{
    /** API value "asc" */
    Ascending,
    /** API value "desc" */
    Descending
};

enum class Details
{
    /** API value "no" */
    NoDetails,
    /** API value "yes" */
    PopulateDetails,
    /** API value "shallow" */
    ShallowDetails
};

enum class Privacy
{
    /** API value "public" */
    Public,
    /** API value "private" */
    Private
};

enum class FileDrop
{
    /** API value "no" */
    Disabled,
    /** API value "yes" */
    Enabled
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

enum class SharedWithUser
{
    /** API value "no" */
    UserIsOwner,
    /** API value "yes" */
    SharedWithUser
};

enum class ChunksRemaining
{
    /** API value "" */
    MoreChunks,
    /** API value "yes" */
    LastChunk
};

/**
 * @class Response
 * @brief Response from API request "folder/get_content"
 */
class Response : public ResponseBase
{
public:
    Response() :
        chunks_remaining(ChunksRemaining::MoreChunks)
    {}
    struct Links
    {
        /** API response field "direct_download" */
        boost::optional<std::string> direct_download;

        /** API response field "edit" */
        boost::optional<std::string> edit;

        /** API response field "normal_download" */
        boost::optional<std::string> normal_download;

        /** API response field "one_time_download" */
        boost::optional<std::string> one_time_download;

        /** API response field "view" */
        boost::optional<std::string> view;
    };
    struct File
    {
        /** API response field "created" */
        boost::posix_time::ptime created_datetime;

        /** API response field "delete_date" */
        boost::posix_time::ptime deleted_datetime;

        /** API response field "description" */
        boost::optional<std::string> description;

        /** API response field "downloads" */
        uint32_t download_count;

        /** API response field "filename" */
        std::string filename;

        /** API response field "size" */
        uint64_t filesize;

        /** API response field "filetype" */
        std::string filetype;

        /** API response field "flag" */
        uint32_t flag;

        /** API response field "hash" */
        std::string hash;

        /** API response field "links" */
        Links links;

        /** API response field "mimetype" */
        std::string mimetype;

        /** API response field "password_protected" */
        PasswordProtected password_protected;

        /** API response field "permissions.value" */
        boost::optional<uint32_t> permissions;

        /** API response field "privacy" */
        Privacy privacy;

        /** API response field "quickkey" */
        std::string quickkey;

        /** API response field "revision" */
        uint32_t revision;

        /** API response field "shared_by_user" */
        SharedByUser shared_by_user;

        /** API response field "shared" */
        SharedWithUser shared_with_user;

        /** API response field "views" */
        uint32_t views;
    };
    struct Folder
    {
        /** API response field "created" */
        boost::posix_time::ptime created_datetime;

        /** API response field "delete_date" */
        boost::posix_time::ptime deleted_datetime;

        /** API response field "description" */
        boost::optional<std::string> description;

        /** API response field "file_count" */
        uint32_t file_count;

        /** API response field "dropbox_enabled" */
        FileDrop filedrop;

        /** API response field "flag" */
        uint32_t flag;

        /** API response field "folder_count" */
        uint32_t folder_count;

        /** API response field "folderkey" */
        std::string folderkey;

        /** API response field "name" */
        std::string name;

        /** API response field "permissions.value" */
        boost::optional<uint32_t> permissions;

        /** API response field "privacy" */
        Privacy privacy;

        /** API response field "revision" */
        uint32_t revision;

        /** API response field "shared_by_user" */
        SharedByUser shared_by_user;

        /** API response field "shared" */
        SharedWithUser shared_with_user;

        /** API response field "tags" */
        boost::optional<std::string> tags;

        /** API response field "total_files" */
        boost::optional<uint32_t> total_files;

        /** API response field "total_folders" */
        boost::optional<uint32_t> total_folders;

        /** API response field "total_size" */
        boost::optional<uint64_t> total_size;
    };
    /** API response field "response.folder_content.chunk_size" */
    uint32_t chunk_size;

    /** API response field "response.folder_content.content_type" */
    ContentType content_type;

    /** API response field "response.folder_content.chunk_number" */
    uint32_t chunk_number;

    /** API response field "response.folder_content.???" */
    ChunksRemaining chunks_remaining;

    /** API response field "response.folder_content.files" */
    std::vector<File> files;

    /** API response field "response.folder_content.folders" */
    std::vector<Folder> folders;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "folder/get_content"
 */
class Request
{
public:
    /**
     * API request "folder/get_content"
     *
     * @param folder_key API parameter "folder_key"
     * @param chunk_number API parameter "chunk"
     * @param content_type API parameter "content_type"
     */
    Request(
            std::string folder_key,
            uint32_t chunk_number,
            ContentType content_type
        );

    /**
     * Optional API parameter "device_id"
     *
     * @param device_id Set parameter "device_id" in API request.
     */
    void SetDeviceId(uint32_t device_id);

    /**
     * Optional API parameter "order_by"
     *
     * @param order_by Set parameter "order_by" in API request.
     */
    void SetOrderBy(OrderBy order_by);

    /**
     * Optional API parameter "order_direction"
     *
     * @param order_direction Set parameter "order_direction" in API request.
     */
    void SetOrderDirection(OrderDirection order_direction);

    /**
     * Optional API parameter "filter"
     *
     * @param filter Set parameter "filter" in API request.
     */
    void SetFilter(std::string filter);

    /**
     * Optional API parameter "details"
     *
     * @param details Set parameter "details" in API request.
     */
    void SetDetails(Details details);

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
}  // namespace v2_0

// The latest version
using namespace v2_0;  // NOLINT

}  // namespace get_content
}  // namespace folder
}  // namespace api
}  // namespace mf
