/**
 * @file api/upload/instant.hpp
 * @brief API request: /api/1.5/upload/instant
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
/** API action path "upload" */
namespace upload {
/** API action "upload/instant" */
namespace instant {
/** API path "/api/1.5/upload/instant" */
namespace v1_5 {

enum class ActionOnDuplicate
{
    /** API value "skip" */
    Skip,
    /** API value "keep" */
    Keep,
    /** API value "replace" */
    Replace
};

/**
 * @class ResponseData
 * @brief Response data from API request "upload/instant"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** API response field "response.quickkey" */
    std::string quickkey;

    /** API response field "response.filename" */
    std::string filename;

    /** API response field "response.new_device_revision" */
    uint32_t new_device_revision;
};

/**
 * @class Response
 * @brief Response from API request "upload/instant"
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
 * @brief Make API request "upload/instant"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using ActionOnDuplicate = ::mf::api::upload::instant::v1_5::ActionOnDuplicate;

    /**
     * API request "upload/instant"
     *
     * @param filename API parameter "filename"
     * @param hash API parameter "hash"
     * @param filesize API parameter "size"
     */
    Request(
            std::string filename,
            std::string hash,
            uint64_t filesize
        );

    /**
     * Optional API parameter "quickkey"
     *
     * @param quickkey Set parameter "quickkey" in API request.
     */
    void SetQuickkey(std::string quickkey);

    /**
     * Optional API parameter "folder_key"
     *
     * @param target_parent_folderkey Set parameter "folder_key" in API request.
     */
    void SetTargetParentFolderkey(std::string target_parent_folderkey);

    /**
     * Optional API parameter "filedrop_key"
     *
     * @param target_filedrop Set parameter "filedrop_key" in API request.
     */
    void SetTargetFiledrop(std::string target_filedrop);

    /**
     * Optional API parameter "path"
     *
     * @param target_path Set parameter "path" in API request.
     */
    void SetTargetPath(std::string target_path);

    /**
     * Optional API parameter "mtime"
     *
     * @param mtime Set parameter "mtime" in API request.
     */
    void SetMtime(boost::posix_time::ptime mtime);

    /**
     * Optional API parameter "action_on_duplicate"
     *
     * @param action_on_duplicate Set parameter "action_on_duplicate" in API
     *                            request.
     */
    void SetActionOnDuplicate(ActionOnDuplicate action_on_duplicate);

    /**
     * Optional API parameter "previous_hash"
     *
     * @param previous_hash Set parameter "previous_hash" in API request.
     */
    void SetPreviousHash(std::string previous_hash);

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
}  // namespace v1_5

}  // namespace instant
}  // namespace upload
}  // namespace api
}  // namespace mf
