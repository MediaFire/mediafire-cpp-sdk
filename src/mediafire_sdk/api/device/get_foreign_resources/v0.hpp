/**
 * @file api/device/get_foreign_resources.hpp
 * @brief API request: /api/device/get_foreign_resources
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
/** API action path "device" */
namespace device {
/** API action "device/get_foreign_resources" */
namespace get_foreign_resources {
/** API path "/api/device/get_foreign_resources" */
namespace v0 {

enum class ContentType
{
    /** API value "folders" */
    Folders,
    /** API value "files" */
    Files
};

enum class ShareLinkEnabled
{
    /** API value "0" */
    LinkDisabled,
    /** API value "1" */
    LinkEnabled
};

enum class SyncToDesktopFilter
{
    /** API value "0" */
    All,
    /** API value "1" */
    DesktopSyncedOnly
};

/**
 * @class ResponseData
 * @brief Response data from API request "device/get_foreign_resources"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    struct File
    {
        /** API response field "attributes" */
        uint32_t attributes;

        /** API response field "created" */
        boost::posix_time::ptime created_datetime;

        /** API response field "date_shared" */
        boost::posix_time::ptime shared_datetime;

        /** API response field "desc" */
        boost::optional<std::string> description;

        /** API response field "filename" */
        std::string filename;

        /** API response field "filetype" */
        std::string filetype;

        /** API response field "md5" */
        std::string hash;

        /** API response field "mimetype" */
        std::string mimetype;

        /** API response field "options" */
        uint32_t options;

        /** API response field "permissions" */
        boost::optional<uint32_t> permissions;

        /** API response field "quickkey" */
        std::string quickkey;

        /** API response field "revision" */
        uint32_t revision;

        /** API response field "share_link_enabled" */
        ShareLinkEnabled share_link_enabled;

        /** API response field "sharer" */
        std::string sharer;

        /** API response field "size" */
        uint64_t filesize;
    };
    struct Folder
    {
        /** API response field "attributes" */
        uint32_t attributes;

        /** API response field "created" */
        boost::posix_time::ptime created_datetime;

        /** API response field "date_shared" */
        boost::posix_time::ptime shared_datetime;

        /** API response field "desc" */
        boost::optional<std::string> description;

        /** API response field "folderkey" */
        std::string folderkey;

        /** API response field "name" */
        std::string name;

        /** API response field "options" */
        uint32_t options;

        /** API response field "permissions" */
        boost::optional<uint32_t> permissions;

        /** API response field "resource_key" */
        boost::optional<std::string> resource_key;

        /** API response field "revision" */
        uint32_t revision;

        /** API response field "share_link_enabled" */
        ShareLinkEnabled share_link_enabled;

        /** API response field "sharer" */
        std::string sharer;
    };
    /** API response field "response.files" */
    std::vector<File> files;

    /** API response field "response.folders" */
    std::vector<Folder> folders;
};

/**
 * @class Response
 * @brief Response from API request "device/get_foreign_resources"
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
 * @brief Make API request "device/get_foreign_resources"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using ContentType = enum ContentType;
    using ShareLinkEnabled = enum ShareLinkEnabled;
    using SyncToDesktopFilter = enum SyncToDesktopFilter;

    /**
     * API request "device/get_foreign_resources"
     */
    Request();

    /**
     * API request "device/get_foreign_resources"
     *
     * Deprecated constructor
     *
     * @param chunk_number API parameter "chunk"
     * @param content_type API parameter "type"
     */
    Request(
            uint32_t chunk_number,
            ContentType content_type
        );

    /**
     * Optional API parameter "chunk"
     *
     * @param chunk_number Set parameter "chunk" in API request.
     */
    void SetChunkNumber(uint32_t chunk_number);

    /**
     * Optional API parameter "type"
     *
     * @param content_type Set parameter "type" in API request.
     */
    void SetContentType(ContentType content_type);

    /**
     * Optional API parameter "filter"
     *
     * @param filter Set parameter "filter" in API request.
     */
    void SetFilter(std::string filter);

    /**
     * Optional API parameter "sync"
     *
     * @param sync_to_desktop_filter Set parameter "sync" in API request.
     */
    void SetSyncToDesktopFilter(SyncToDesktopFilter sync_to_desktop_filter);

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

}  // namespace get_foreign_resources
}  // namespace device
}  // namespace api
}  // namespace mf
