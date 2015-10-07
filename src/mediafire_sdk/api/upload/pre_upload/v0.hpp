/**
 * @file api/upload/pre_upload.hpp
 * @brief API request: /api/upload/pre_upload
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
/** API action "upload/pre_upload" */
namespace pre_upload {
/** API path "/api/upload/pre_upload" */
namespace v0 {

enum class Resumability
{
    /** API value "no" */
    NotResumable,
    /** API value "yes" */
    Resumable
};

enum class ActionOnDuplicate
{
    /** API value "skip" */
    Skip,
    /** API value "keep" */
    Keep,
    /** API value "replace" */
    Replace
};

enum class StorageLimit
{
    /** API value "no" */
    WithinBounds,
    /** API value "yes" */
    Exceeded
};

enum class DuplicateName
{
    /** API value "no" */
    NoPreexistingName,
    /** API value "yes" */
    DuplicateNameExists
};

enum class UnitsReady
{
    /** API value "no" */
    NotReady,
    /** API value "yes" */
    AllUnitsReady
};

enum class KnownByCloud
{
    /** API value "no" */
    Known,
    /** API value "yes" */
    Unknown
};

/**
 * @class ResponseData
 * @brief Response data from API request "upload/pre_upload"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** API response field "response.storage_limit_exceeded" */
    boost::optional<StorageLimit> storage_limit_exceeded;

    /** API response field "response.storage_limit" */
    boost::optional<uint64_t> storage_limit;

    /** API response field "response.used_storage_size" */
    boost::optional<uint64_t> used_storage_size;

    /** API response field "response.new_hash" */
    boost::optional<KnownByCloud> known_by_cloud;

    /** API response field "response.quickkey" */
    boost::optional<std::string> quickkey;

    /** API response field "response.duplicate_quickkey" */
    boost::optional<std::string> duplicate_quickkey;

    /** API response field "response.duplicate_name" */
    boost::optional<DuplicateName> duplicate_name;

    /** API response field "response.resumable_upload.upload_key" */
    boost::optional<std::string> upload_key;

    /** API response field "response.resumable_upload.all_units_ready" */
    boost::optional<UnitsReady> all_units_ready;

    /** API response field "response.resumable_upload.number_of_units" */
    boost::optional<uint32_t> number_of_units;

    /** API response field "response.resumable_upload.unit_size" */
    boost::optional<uint64_t> unit_size;

    /** API response field "response.resumable_upload.bitmap.count" */
    boost::optional<uint32_t> bitmap_count;

    /** API response field "response.resumable_upload.bitmap.words" */
    std::vector<uint16_t> bitmap_words;
};

/**
 * @class Response
 * @brief Response from API request "upload/pre_upload"
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
 * @brief Make API request "upload/pre_upload"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Resumability = ::mf::api::upload::pre_upload::v0::Resumability;
    using ActionOnDuplicate = ::mf::api::upload::pre_upload::v0::ActionOnDuplicate;
    using StorageLimit = ::mf::api::upload::pre_upload::v0::StorageLimit;
    using DuplicateName = ::mf::api::upload::pre_upload::v0::DuplicateName;
    using UnitsReady = ::mf::api::upload::pre_upload::v0::UnitsReady;
    using KnownByCloud = ::mf::api::upload::pre_upload::v0::KnownByCloud;

    /**
     * API request "upload/pre_upload"
     */
    Request();

    /**
     * Optional API parameter "resumable"
     *
     * @param resumable Set parameter "resumable" in API request.
     */
    void SetResumable(Resumability resumable);

    /**
     * Optional API parameter "filename"
     *
     * @param filename Set parameter "filename" in API request.
     */
    void SetFilename(std::string filename);

    /**
     * Optional API parameter "quick_key"
     *
     * @param quickkey Set parameter "quick_key" in API request.
     */
    void SetQuickkey(std::string quickkey);

    /**
     * Optional API parameter "hash"
     *
     * @param hash Set parameter "hash" in API request.
     */
    void SetHash(std::string hash);

    /**
     * Optional API parameter "size"
     *
     * @param filesize Set parameter "size" in API request.
     */
    void SetFilesize(uint64_t filesize);

    /**
     * Optional API parameter "upload_folder_key"
     *
     * @param upload_folder_key Set parameter "upload_folder_key" in API
     *                          request.
     */
    void SetUploadFolderKey(std::string upload_folder_key);

    /**
     * Optional API parameter "path"
     *
     * @param relative_path Set parameter "path" in API request.
     */
    void SetRelativePath(std::string relative_path);

    /**
     * Optional API parameter "action_on_duplicate"
     *
     * @param action_on_duplicate Set parameter "action_on_duplicate" in API
     *                            request.
     */
    void SetActionOnDuplicate(ActionOnDuplicate action_on_duplicate);

    /**
     * Optional API parameter "mtime"
     *
     * @param mtime Set parameter "mtime" in API request.
     */
    void SetMtime(boost::posix_time::ptime mtime);

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

}  // namespace pre_upload
}  // namespace upload
}  // namespace api
}  // namespace mf
