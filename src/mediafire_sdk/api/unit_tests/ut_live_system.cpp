/**
 * @file api/unit_tests/ut_live_system.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/system/get_limits.hpp"
#include "mediafire_sdk/api/system/get_status.hpp"
#include "mediafire_sdk/api/system/get_version.hpp"
#include "mediafire_sdk/api/system/get_info.hpp"
#include "mediafire_sdk/api/system/get_editable_media.hpp"
#include "mediafire_sdk/api/system/get_supported_media.hpp"
#include "mediafire_sdk/api/system/get_mime_types.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveSystem
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

BOOST_FIXTURE_TEST_SUITE(s, ut::Fixture)

BOOST_AUTO_TEST_CASE(SystemGetLimits)
{
    Call(api::system::get_limits::Request(),
         [&](const api::system::get_limits::Response & response)
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

BOOST_AUTO_TEST_CASE(SystemGetStatus)
{
    Call(api::system::get_status::Request(),
         [&](const api::system::get_status::Response & response)
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

BOOST_AUTO_TEST_CASE(SystemGetVersion)
{
    Call(api::system::get_version::Request(),
         [&](const api::system::get_version::Response & response)
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

BOOST_AUTO_TEST_CASE(SystemGetInfo)
{
    Call(api::system::get_info::Request(),
         [&](const api::system::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & data = *response.response_data;

                 bool non_empty = false;
                 for (auto & str : data.viewable_extensions)
                 {
                     if (!str.empty())
                         non_empty = true;
                 }
                 if (non_empty)
                 {
                     Success();
                 }
                 else
                 {
                     std::cout << "Only empty items in viewable_extensions!"
                               << std::endl;
                     Fail(response);
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(SystemGetSupportedMedia)
{
    Call(api::system::get_supported_media::Request(),
         [&](const api::system::get_supported_media::Response & response)
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

BOOST_AUTO_TEST_CASE(SystemGetEditableMedia)
{
    Call(api::system::get_editable_media::Request(),
         [&](const api::system::get_editable_media::Response & response)
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

BOOST_AUTO_TEST_CASE(SystemGetMimeTypes)
{
    Call(api::system::get_mime_types::Request(),
         [&](const api::system::get_mime_types::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else if (response.response_data->mimetypes.empty())
             {
                 Fail("Empty mime type list.");
             }
             else
             {
                 Success();
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
