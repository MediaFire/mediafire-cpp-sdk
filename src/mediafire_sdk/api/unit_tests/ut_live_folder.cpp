/**
 * @file api/unit_tests/ut_live_folder.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/folder/copy.hpp"
#include "mediafire_sdk/api/folder/create.hpp"
#include "mediafire_sdk/api/folder/folder_delete.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"
#include "mediafire_sdk/api/folder/get_info.hpp"
#include "mediafire_sdk/api/folder/move.hpp"
#include "mediafire_sdk/api/folder/update.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveFolder
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

BOOST_FIXTURE_TEST_SUITE(s, ut::Fixture)

BOOST_AUTO_TEST_CASE(FolderGetContentLive)
{
    api::folder::get_content::Request get_content(
            "myfiles",                                    // folder_key
            0,                                            // chunk
            api::folder::get_content::ContentType::Files  // content_type
            );

    Call(get_content, [&](const api::folder::get_content::Response & response)
         {
             Stop();

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

BOOST_AUTO_TEST_CASE(FolderGetInfoMyFilesBlank)
{
    Call(api::folder::get_info::Request(""),
         [&](const api::folder::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & data = *response.response_data;

                 Success();

                 BOOST_CHECK(!data.folderkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderGetInfoMyFilesExplicit)
{
    Call(api::folder::get_info::Request("myfiles"),
         [&](const api::folder::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & data = *response.response_data;

                 Success();

                 BOOST_CHECK(!data.folderkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
