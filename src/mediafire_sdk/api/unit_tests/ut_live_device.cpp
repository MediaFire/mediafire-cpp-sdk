/**
 * @file api/unit_tests/ut_live_device.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/device/get_changes.hpp"
#include "mediafire_sdk/api/device/get_status.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveDevice
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

namespace globals
{
using namespace ut::globals;
uint32_t known_revision = 0;
}  // namespace globals

BOOST_FIXTURE_TEST_SUITE(s, ut::Fixture)

/**
 * Get the device revision for the next text
 */
BOOST_AUTO_TEST_CASE(DeviceGetStatus)
{
    Call(api::device::get_status::Request(),
         [&](const api::device::get_status::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & data = *response.response_data;
                 globals::known_revision = data.device_revision;
                 Success();
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeviceGetChanges)
{
    // Get revision, starting at multiple of 500
    uint32_t revision = globals::known_revision;
    revision -= revision % 500;

    Call(api::device::get_changes::Request(revision),
         [&](const api::device::get_changes::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Success();
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
