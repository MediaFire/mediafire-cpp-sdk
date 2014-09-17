/**
 * @file api/device/get_changes.hpp
 * @brief API request: /api/device/get_changes
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
/** API action "device/get_changes" */
namespace get_changes {
/** API path "/api/device/get_changes" */
namespace v0 {

/**
 * @class Response
 * @brief Response from API request "device/get_changes"
 */
class Response : public ResponseBase
{
public:
    struct File
    {
        /** API response field "quickkey" */
        std::string quickkey;

        /** API response field "parent_folderkey" */
        boost::optional<std::string> parent_folderkey;

        /** API response field "revision" */
        uint32_t revision;

        /** API response field "created" */
        boost::posix_time::ptime created_datetime;
    };
    struct Folder
    {
        /** API response field "folderkey" */
        std::string folderkey;

        /** API response field "parent_folderkey" */
        boost::optional<std::string> parent_folderkey;

        /** API response field "revision" */
        uint32_t revision;

        /** API response field "created" */
        boost::posix_time::ptime created_datetime;
    };
    /** API response field "response.device_revision" */
    uint32_t device_revision;

    /** API response field "response.changes_list_block" */
    uint32_t changes_list_block;

    /** API response field "response.updated.files" */
    std::vector<File> updated_files;

    /** API response field "response.deleted.files" */
    std::vector<File> deleted_files;

    /** API response field "response.updated.folders" */
    std::vector<Folder> updated_folders;

    /** API response field "response.deleted.folders" */
    std::vector<Folder> deleted_folders;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "device/get_changes"
 */
class Request
{
public:
    /**
     * API request "device/get_changes"
     *
     * @param revision API parameter "revision"
     */
    explicit Request(
            uint32_t revision
        );

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

// The latest version
using namespace v0;  // NOLINT

}  // namespace get_changes
}  // namespace device
}  // namespace api
}  // namespace mf