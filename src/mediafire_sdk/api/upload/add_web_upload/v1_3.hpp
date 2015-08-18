/**
 * @file api/upload/add_web_upload.hpp
 * @brief API request: /api/1.3/upload/add_web_upload
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
/** API action "upload/add_web_upload" */
namespace add_web_upload {
/** API path "/api/1.3/upload/add_web_upload" */
namespace v1_3 {

/**
 * @class ResponseData
 * @brief Response data from API request "upload/add_web_upload"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** An alpha-numeric key used with upload/poll_upload to determine the
     * status of the file upload. */
    std::string upload_key;
};

/**
 * @class Response
 * @brief Response from API request "upload/add_web_upload"
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
 * @brief Make API request "upload/add_web_upload"
 */
class Request
{
public:
    /**
     * API request "upload/add_web_upload"
     *
     * @param url The URL to the file to be saved.
     * @param filename The name plus extension of the file to be created. The
     *                 name should be 3 to 255 characters in length.
     */
    Request(
            std::string url,
            std::string filename
        );

    /**
     * Optional API parameter "folder_key"
     *
     * @param folder_key The destination folder key. If not passed the API will
     *                   use the cloud root folder.
     */
    void SetFolderKey(std::string folder_key);

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

}  // namespace add_web_upload
}  // namespace upload
}  // namespace api
}  // namespace mf
