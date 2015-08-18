/**
 * @file api/unit_tests/ut_live_billing.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/billing/get_products.hpp"
#include "mediafire_sdk/api/billing/get_plans.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveBilling
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

BOOST_FIXTURE_TEST_SUITE(s, ut::Fixture)

BOOST_AUTO_TEST_CASE(GetBillingProducts)
{
    api::billing::get_products::Request request;
    request.SetActive(api::billing::get_products::Activity::Active);

    Call(request, [&](const api::billing::get_products::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & data = *response.response_data;

                 Success();

                 BOOST_CHECK(!data.products.empty());

                 for (const api::billing::get_products::ResponseData::Product &
                              it : data.products)
                 {
                     BOOST_CHECK(!it.description.empty());
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetBillingPlans)
{
    Call(api::billing::get_plans::Request(),
         [&](const api::billing::get_plans::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & data = *response.response_data;

                 Success();

                 BOOST_CHECK(!data.plans.empty());

                 for (const api::billing::get_plans::ResponseData::Plan & it :
                      data.plans)
                 {
                     BOOST_CHECK(!it.description.empty());
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
