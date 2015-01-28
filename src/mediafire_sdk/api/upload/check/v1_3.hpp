/**
 * @file api/upload/check.hpp
 * @brief API request: /api/1.3/upload/check
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
/** API action "upload/check" */
namespace check {
/** API path "/api/1.3/upload/check" */
namespace v1_3 {

enum class Resumable
{
    /** API value "no" */
    NotResumable,
    /** API value "yes" */
    Resumable
};

enum class HashAlreadyInSystem
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

enum class HashAlreadyInAccount
{
    /** API value "no" */
    HashNewToAccount,
    /** API value "yes" */
    HashExistsInAccount
};

enum class HashAlreadyInFolder
{
    /** API value "no" */
    HashNewToFolder,
    /** API value "yes" */
    HashExistsInFolder
};

enum class FilenameInFolder
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

enum class FileExistsWithDifferentHash
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

enum class StorageLimitExceeded
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
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

enum class VersionControl
{
    /** API value "keep_revision" */
    KeepRevision,
    /** API value "none" */
    Required
};

enum class AllUnitsReady
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

/**
 * @class Response
 * @brief Response from API request "upload/check"
 */
class Response : public ResponseBase
{
public:
    Response() :
        hash_exists(HashAlreadyInSystem::No),
        hash_in_account(HashAlreadyInAccount::HashNewToAccount),
        hash_in_folder(HashAlreadyInFolder::HashNewToFolder),
        file_exists(FilenameInFolder::No),
        hash_different(FileExistsWithDifferentHash::No),
        available_space(0),
        used_storage_size(0),
        storage_limit(0),
        storage_limit_exceeded(StorageLimitExceeded::No)
    {}
    struct ResumableData
    {
        /** API response field "number_of_units" */
        uint32_t number_of_units;

        /** API response field "all_units_ready" */
        AllUnitsReady all_units_ready;

        /** API response field "unit_size" */
        uint64_t unit_size;

        /** API response field "bitmap.count" */
        boost::optional<uint32_t> bitmap_count;

        /** API response field "bitmap.words" */
        std::vector<uint16_t> words;

        /** API response field "upload_key" */
        boost::optional<std::string> upload_key;
    };
    /** API response field "response.hash_exists" */
    HashAlreadyInSystem hash_exists;

    /** API response field "response.in_account" */
    HashAlreadyInAccount hash_in_account;

    /** API response field "response.in_folder" */
    HashAlreadyInFolder hash_in_folder;

    /** API response field "response.file_exists" */
    FilenameInFolder file_exists;

    /** API response field "response.different_hash" */
    FileExistsWithDifferentHash hash_different;

    /** API response field "response.duplicate_quickkey" */
    boost::optional<std::string> duplicate_quickkey;

    /** API response field "response.available_space" */
    uint64_t available_space;

    /** API response field "response.used_storage_size" */
    uint64_t used_storage_size;

    /** API response field "response.storage_limit" */
    uint64_t storage_limit;

    /** API response field "response.storage_limit_exceeded" */
    StorageLimitExceeded storage_limit_exceeded;

    /** API response field "response.resumable_upload" */
    boost::optional<ResumableData> resumable;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "upload/check"
 */
class Request
{
public:
    /**
     * API request "upload/check"
     *
     * @param filename API parameter "filename"
     */
    explicit Request(
            std::string filename
        );

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
     * @param path Set parameter "path" in API request.
     */
    void SetPath(std::string path);

    /**
     * Optional API parameter "resumable"
     *
     * @param resumable Set parameter "resumable" in API request.
     */
    void SetResumable(Resumable resumable);

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
}  // namespace v1_3

}  // namespace check
}  // namespace upload
}  // namespace api
}  // namespace mf