/**
 * @file api/billing/cancel_plan.hpp
 * @brief API request: /api/1.1/billing/cancel_plan
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
/** API action path "billing" */
namespace billing {
/** API action "billing/cancel_plan" */
namespace cancel_plan {
/** API path "/api/1.1/billing/cancel_plan" */
namespace v1_1 {

enum class CancellationReason
{
    /** API value "0" */
    No,
    /** API value "1" */
    Yes
};

/**
 * @class ResponseData
 * @brief Response data from API request "billing/cancel_plan"
 *
 * This data is only available if the API request was successful.
 */
class ResponseData
{
public:
    /** API response field "response.cancel_date" */
    boost::posix_time::ptime cancel_date;

    /** API response field "response.confirmation" */
    std::string confirmation;

    /** API response field "response.last_invoice" */
    boost::optional<std::string> last_invoice;
};

/**
 * @class Response
 * @brief Response from API request "billing/cancel_plan"
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
 * @brief Make API request "billing/cancel_plan"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using CancellationReason = ::mf::api::billing::cancel_plan::v1_1::CancellationReason;

    /**
     * API request "billing/cancel_plan"
     */
    Request();

    /**
     * Optional API parameter "comments"
     *
     * @param comments Set parameter "comments" in API request.
     */
    void SetComments(std::string comments);

    /**
     * Optional API parameter "x_uploading"
     *
     * @param uploading_problems Set parameter "x_uploading" in API request.
     */
    void SetUploadingProblems(CancellationReason uploading_problems);

    /**
     * Optional API parameter "x_downloading"
     *
     * @param downloading_problems Set parameter "x_downloading" in API request.
     */
    void SetDownloadingProblems(CancellationReason downloading_problems);

    /**
     * Optional API parameter "x_switching"
     *
     * @param switching_to_other_service Set parameter "x_switching" in API
     *                                   request.
     */
    void SetSwitchingToOtherService(CancellationReason switching_to_other_service);

    /**
     * Optional API parameter "x_speed"
     *
     * @param download_speed_insufficient Set parameter "x_speed" in API
     *                                    request.
     */
    void SetDownloadSpeedInsufficient(CancellationReason download_speed_insufficient);

    /**
     * Optional API parameter "x_money_problems"
     *
     * @param out_of_money Set parameter "x_money_problems" in API request.
     */
    void SetOutOfMoney(CancellationReason out_of_money);

    /**
     * Optional API parameter "x_too_expensive"
     *
     * @param too_expensive Set parameter "x_too_expensive" in API request.
     */
    void SetTooExpensive(CancellationReason too_expensive);

    /**
     * Optional API parameter "x_technophobe"
     *
     * @param no_longer_needed Set parameter "x_technophobe" in API request.
     */
    void SetNoLongerNeeded(CancellationReason no_longer_needed);

    /**
     * Optional API parameter "x_lacking_features"
     *
     * @param lacking_features Set parameter "x_lacking_features" in API
     *                         request.
     */
    void SetLackingFeatures(CancellationReason lacking_features);

    /**
     * Optional API parameter "x_lacking_service"
     *
     * @param lacking_service Set parameter "x_lacking_service" in API request.
     */
    void SetLackingService(CancellationReason lacking_service);

    /**
     * Optional API parameter "x_other"
     *
     * @param cc_expiration Set parameter "x_other" in API request.
     */
    void SetCcExpiration(std::string cc_expiration);

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
}  // namespace v1_1

}  // namespace cancel_plan
}  // namespace billing
}  // namespace api
}  // namespace mf
