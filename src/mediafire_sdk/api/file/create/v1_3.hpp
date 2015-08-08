/**
 * @file api/file/create.hpp
 * @brief API request: /api/1.3/file/create
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
/** API action "file/create" */
namespace create {
/** API path "/api/1.3/file/create" */
namespace v1_3 {

enum class FileType
{
    /** API value "" */
    Default,
    /** API value "text" */
    Text
};

enum class PasswordProtected
{
    /** API value "no" */
    Unprotected,
    /** API value "yes" */
    Protected
};

enum class Privacy
{
    /** API value "public" */
    Public,
    /** API value "private" */
    Private
};

/**
 * @class ResponseData
 * @brief Response data from API request "file/create"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        description(""),
        mimetype("")
    {}
    struct Links
    {
        /** API response field "direct_download" */
        boost::optional<std::string> direct_download;

        /** API response field "edit" */
        boost::optional<std::string> edit;

        /** API response field "listen" */
        boost::optional<std::string> listen;

        /** API response field "normal_download" */
        boost::optional<std::string> normal_download;

        /** API response field "one_time.download" */
        boost::optional<std::string> one_time_download;

        /** API response field "one_time.listen" */
        boost::optional<std::string> one_time_listen;

        /** API response field "one_time.watch" */
        boost::optional<std::string> one_time_watch;

        /** API response field "one_time.view" */
        boost::optional<std::string> one_time_view;

        /** API response field "read" */
        boost::optional<std::string> read;

        /** API response field "streaming" */
        boost::optional<std::string> streaming;

        /** API response field "view" */
        boost::optional<std::string> view;

        /** API response field "watch" */
        boost::optional<std::string> watch;
    };
    /** API response field "response.fileinfo.created" */
    boost::posix_time::ptime created_datetime;

    /** API response field "response.fileinfo.description" */
    std::string description;

    /** API response field "response.new_device_revision" */
    boost::optional<uint32_t> new_device_revision;

    /** API response field "response.fileinfo.downloads" */
    uint32_t downloads;

    /** API response field "response.fileinfo.hash" */
    std::string filehash;

    /** API response field "response.fileinfo.filename" */
    std::string filename;

    /** API response field "response.fileinfo.size" */
    uint64_t filesize;

    /** API response field "response.fileinfo.filetype" */
    std::string filetype;

    /** API response field "response.fileinfo.revision" */
    uint32_t file_revision;

    /** API response field "response.fileinfo.flag" */
    uint32_t flag;

    /** API response field "response.fileinfo.links" */
    Links links;

    /** API response field "response.fileinfo.mimetype" */
    std::string mimetype;

    /** API response field "response.fileinfo.password_protected" */
    PasswordProtected password_protected;

    /** API response field "response.fileinfo.privacy" */
    Privacy privacy;

    /** API response field "response.fileinfo.quickkey" */
    std::string quickkey;
};

/**
 * @class Response
 * @brief Response from API request "file/create"
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
 * @brief Make API request "file/create"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using FileType = enum FileType;
    using PasswordProtected = enum PasswordProtected;
    using Privacy = enum Privacy;

    /**
     * API request "file/create"
     */
    Request();

    /**
     * Optional API parameter "filename"
     *
     * @param filename Set parameter "filename" in API request.
     */
    void SetFilename(std::string filename);

    /**
     * Optional API parameter "file_extension"
     *
     * @param file_extension Set parameter "file_extension" in API request.
     */
    void SetFileExtension(std::string file_extension);

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
     * Optional API parameter "type"
     *
     * @param file_type Set parameter "type" in API request.
     */
    void SetFileType(FileType file_type);

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
}  // namespace v1_3

}  // namespace create
}  // namespace file
}  // namespace api
}  // namespace mf
