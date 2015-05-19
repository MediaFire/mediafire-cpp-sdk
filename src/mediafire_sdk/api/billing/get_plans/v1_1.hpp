/**
 * @file api/billing/get_plans.hpp
 * @brief API request: /api/1.1/billing/get_plans
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
/** API action "billing/get_plans" */
namespace get_plans {
/** API path "/api/1.1/billing/get_plans" */
namespace v1_1 {

enum class Recurring
{
    /** API value "0" */
    NonRecurring,
    /** API value "1" */
    Recurring
};

/**
 * @class Response
 * @brief Response from API request "billing/get_plans"
 */
class Response : public ResponseBase
{
public:
    struct Plan
    {
        /** API response field "product_id" */
        uint32_t product_id;

        /** API response field "description" */
        std::string description;

        /** API response field "short_description" */
        std::string short_description;

        /** API response field "initial_amount" */
        std::string initial_amount;

        /** API response field "initial_units" */
        uint64_t initial_units;

        /** API response field "is_recurring" */
        Recurring is_recurring;

        /** API response field "recurring_amount" */
        std::string recurring_amount;

        /** API response field "recurring_units" */
        uint32_t recurring_units;

        /** API response field "frequency_text" */
        std::string frequency_text;

        /** API response field "product_class" */
        uint32_t product_class;

        /** API response field "free_months" */
        uint32_t free_months;

        /** API response field "base_storage" */
        uint64_t base_storage;
    };
    /** API response field "response.products" */
    std::vector<Plan> plans;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "billing/get_plans"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Recurring = enum Recurring;

    /**
     * API request "billing/get_plans"
     */
    Request();

    /**
     * Optional API parameter "product_id"
     *
     * @param product_id Set parameter "product_id" in API request.
     */
    void SetProductId(uint32_t product_id);

    /**
     * Optional API parameter "family"
     *
     * @param family Set parameter "family" in API request.
     */
    void SetFamily(uint32_t family);

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

private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v1_1

}  // namespace get_plans
}  // namespace billing
}  // namespace api
}  // namespace mf
