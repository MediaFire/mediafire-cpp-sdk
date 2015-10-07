/**
 * @file api/user/get_settings.hpp
 * @brief API request: /api/1.3/user/get_settings
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
/** API action path "user" */
namespace user {
/** API action "user/get_settings" */
namespace get_settings {
/** API path "/api/1.3/user/get_settings" */
namespace v1_3 {

enum class DownloadPage
{
    /** API value "no" */
    Skipped,
    /** API value "yes" */
    NotSkipped
};

enum class EmailValidation
{
    /** API value "no" */
    NeedsValidation,
    /** API value "yes" */
    Validated
};

enum class InstantUploads
{
    /** API value "no" */
    Disabled,
    /** API value "yes" */
    Enabled
};

enum class AutoBandwidth
{
    /** API value "no" */
    NoAction,
    /** API value "yes" */
    PurchaseMoreAutomatically
};

enum class StorageLimit
{
    /** API value "no" */
    InBounds,
    /** API value "yes" */
    Exceeded
};

enum class ShareLinkStatus
{
    /** API value "disabled" */
    Disabled,
    /** API value "enabled" */
    Enabled,
    /** API value "inherit" */
    Inherit
};

enum class CollectMetadata
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

/**
 * @class ResponseData
 * @brief Response data from API request "user/get_settings"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    ResponseData() :
        email_address_validated(EmailValidation::NeedsValidation),
        instant_uploads(InstantUploads::Disabled),
        show_download_page_me_from_me(DownloadPage::NotSkipped),
        show_download_page_me_from_others(DownloadPage::NotSkipped),
        show_download_page_others_from_me(DownloadPage::NotSkipped),
        auto_bandwidth(AutoBandwidth::NoAction),
        used_storage_size(0),
        storage_limit(0),
        storage_limit_exceeded(StorageLimit::InBounds),
        previous_file_versions(0),
        share_link_status(ShareLinkStatus::Inherit),
        collect_metadata(CollectMetadata::No)
    {}
    /** Indicates whether the user has validated their email address. */
    EmailValidation email_address_validated;

    /** Indicates the availability of performing "instant" uploads. */
    InstantUploads instant_uploads;

    /** Show the download page when I am logged in and downloading from myself.
     */
    DownloadPage show_download_page_me_from_me;

    /** Show the download page when I am logged in and downloading from someone
     * else. */
    DownloadPage show_download_page_me_from_others;

    /** Show the download page when someone else, logged in or our, downloads
     * from me. */
    DownloadPage show_download_page_others_from_me;

    /** Automatically purchase more bandwidth when bandwidth is low. */
    AutoBandwidth auto_bandwidth;

    /** The amount of storage space, in bytes, the session user's files are
     * currently occupying. */
    uint64_t used_storage_size;

    /** The total amount of storage space, in bytes, available to the user. */
    uint64_t storage_limit;

    /** Indicates if the session user's files are occupying more space than is
     * available to the user. */
    StorageLimit storage_limit_exceeded;

    /** The number of older file versions to keep for a file. */
    uint32_t previous_file_versions;

    /** Specifies the initial share link status for a new file. */
    ShareLinkStatus share_link_status;

    /** Whether or not to collect the meta data for uploaded files. */
    CollectMetadata collect_metadata;
};

/**
 * @class Response
 * @brief Response from API request "user/get_settings"
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
 * @brief Make API request "user/get_settings"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using DownloadPage = ::mf::api::user::get_settings::v1_3::DownloadPage;
    using EmailValidation = ::mf::api::user::get_settings::v1_3::EmailValidation;
    using InstantUploads = ::mf::api::user::get_settings::v1_3::InstantUploads;
    using AutoBandwidth = ::mf::api::user::get_settings::v1_3::AutoBandwidth;
    using StorageLimit = ::mf::api::user::get_settings::v1_3::StorageLimit;
    using ShareLinkStatus = ::mf::api::user::get_settings::v1_3::ShareLinkStatus;
    using CollectMetadata = ::mf::api::user::get_settings::v1_3::CollectMetadata;

    /**
     * API request "user/get_settings"
     */
    Request();

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

}  // namespace get_settings
}  // namespace user
}  // namespace api
}  // namespace mf
