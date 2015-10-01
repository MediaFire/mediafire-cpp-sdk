/**
 * @file api/folder/copy.hpp
 * @brief API request: /api/folder/copy
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
/** API action "folder/copy" */
namespace copy {
/** API path "/api/folder/copy" */
namespace v0 {

enum class Asynchronous
{
    /** API value "no" */
    Synchronous,
    /** API value "yes" */
    Asynchronous
};

/**
 * @class ResponseData
 * @brief Response data from API request "folder/copy"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        asynchronous(Asynchronous::Synchronous)
    {}
    /** API response field "response.asynchronous" */
    Asynchronous asynchronous;

    /** API response field "response.device_revision" */
    uint32_t device_revision;

    /** API response field "response.new_folderkeys" */
    std::string folderkey;
};

/**
 * @class Response
 * @brief Response from API request "folder/copy"
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
 * @brief Make API request "folder/copy"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Asynchronous = Asynchronous;

    /**
     * API request "folder/copy"
     *
     * @param folderkey API parameter "folder_key_src"
     * @param target_parent_folderkey API parameter "folder_key_dst"
     */
    Request(
            std::string folderkey,
            std::string target_parent_folderkey
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

}  // namespace copy
}  // namespace folder
}  // namespace api
}  // namespace mf
