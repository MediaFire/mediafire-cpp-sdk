/**
 * @file api/unit_tests/ut_live_general.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <fstream>

#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/device/get_changes.hpp"
#include "mediafire_sdk/api/device/get_status.hpp"

#include "mediafire_sdk/api/file/configure_one_time_key.hpp"
#include "mediafire_sdk/api/file/copy.hpp"
#include "mediafire_sdk/api/file/create.hpp"
#include "mediafire_sdk/api/file/file_delete.hpp"
#include "mediafire_sdk/api/file/get_info.hpp"
#include "mediafire_sdk/api/file/get_links.hpp"
#include "mediafire_sdk/api/file/get_versions.hpp"
#include "mediafire_sdk/api/file/move.hpp"
#include "mediafire_sdk/api/file/one_time_key.hpp"
#include "mediafire_sdk/api/file/purge.hpp"
#include "mediafire_sdk/api/file/recently_modified.hpp"
#include "mediafire_sdk/api/file/restore.hpp"
#include "mediafire_sdk/api/file/update.hpp"
#include "mediafire_sdk/api/file/update_file.hpp"

#include "mediafire_sdk/api/folder/copy.hpp"
#include "mediafire_sdk/api/folder/create.hpp"
#include "mediafire_sdk/api/folder/folder_delete.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"
#include "mediafire_sdk/api/folder/get_info.hpp"
#include "mediafire_sdk/api/folder/move.hpp"
#include "mediafire_sdk/api/folder/update.hpp"

#include "mediafire_sdk/utils/md5_hasher.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/algorithm/string/find.hpp"
#include "boost/algorithm/string/join.hpp"

#include "boost/format.hpp"

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "mediafire_sdk/api/unit_tests/session_token_test_server.hpp"
#include "mediafire_sdk/api/unit_tests/api_ut_helpers.hpp"

// File upload includes START
#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/uploader/upload_manager.hpp"
#include "mediafire_sdk/uploader/upload_status.hpp"
#include "boost/variant/get.hpp"
// File upload includes END

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLive
#include "boost/test/unit_test.hpp"

namespace posix_time = boost::posix_time;
namespace api = mf::api;

#ifdef MF_UNIT_TEST_LONG_WAIT
const auto skRevisionTimeout = boost::posix_time::minutes(20);
#else
const auto skRevisionTimeout = boost::posix_time::seconds(0);
#endif

namespace globals
{
using namespace ut::globals;

// Enable to disable deletes.
bool disable_deletes = false;

std::string test_folderkey;
std::string test_folder_name;

std::string test_folderkey2;
std::string test_folder_name2;

std::string test_folderkey3;
std::string test_folder_name3;

std::string test_quickkey;
std::string test_file_name;

std::string test_quickkey2;
std::string test_file_name2;

std::string foreign_folderkey;
std::string foreign_folder_name;

uint32_t known_revision = 0;
bool has_updated_files = false;
bool has_updated_folders = false;
bool has_deleted_files = false;
bool has_deleted_folders = false;

bool multiple_revision_success = false;

namespace upload
{
std::string random_file_name;
std::string quickkey_1;
std::string quickkey_2;
std::string hash_1;
std::string hash_2;
bool file_restored = false;
uint32_t upload_revision_1;
uint32_t upload_revision_2;
bool successful_replacement = false;
}  // namespace upload
namespace one_time_key
{
std::string download_token;
}  // namespace one_time_key
namespace copy_move
{
int files = 0;
int folders = 0;
}  // namespace copymove
}  // namespace globals

std::string RandomFilename()
{
    static std::string chars(
            "abcdefghijklmnopqrstuvwxyz"
            "1234567890");

    int name_size = 8;

    static boost::random::random_device rng;
    static boost::random::uniform_int_distribution<> index_dist(
            0, chars.size() - 1);

    std::string data;
    data.reserve(name_size);

    for (int i = 0; i < name_size; ++i)
    {
        data.push_back(chars[index_dist(rng)]);
    }

    data += ".dat";

    return data;
}

// File of random data that deletes itself when it is destroyed
class RandomFile
{
public:
    RandomFile(uint32_t size) : size_(size), name_(RandomFilename())
    {
        using std::ios;

        std::ofstream file_handle;

        file_handle.open(name_.c_str(), ios::binary | ios::out);
        if (file_handle.is_open() && file_handle.good())
        {
            static std::string chars(
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "1234567890"
                    "!@#$%^&*()"
                    "`~-_=+[{]{\\|;:'\",<.>/? ");

            static boost::random::random_device rng;
            static boost::random::uniform_int_distribution<> index_dist(
                    0, chars.size() - 1);

            for (std::size_t i = 0; i < size_; ++i)
                file_handle.put(chars[index_dist(rng)]);
        }
    }

    ~RandomFile()
    {
        boost::system::error_code ec;
        boost::filesystem::remove(name_, ec);
    }

    const std::string & Name() const { return name_; }

private:
    uint32_t size_;
    std::string name_;
};

BOOST_FIXTURE_TEST_SUITE(s, ut::Fixture)

BOOST_AUTO_TEST_CASE(_WARNING_)
{
    if (globals::disable_deletes)
        Debug("Deletes are disabled!");
}

/**
 * UNIFIED OPERATIONS
 *
 * This section does a few file and folder operations and ensures that the
 * device revision is updated.
 */

BOOST_AUTO_TEST_CASE(CreateFolder)
{
    globals::test_folder_name = ut::RandomAlphaNum(20);

    Debug(globals::test_folder_name);

    Call(api::folder::create::Request(globals::test_folder_name),
         [&](const api::folder::create::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::test_folderkey = response_data.folderkey;

                 BOOST_CHECK(!response_data.folderkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFolder2)
{
    globals::test_folder_name2 = ut::RandomAlphaNum(20);

    Debug(globals::test_folder_name2);

    Call(api::folder::create::Request(globals::test_folder_name2),
         [&](const api::folder::create::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::test_folderkey2 = response_data.folderkey;

                 BOOST_CHECK(!response_data.folderkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderGetInfo)
{
    Call(api::folder::get_info::Request(globals::test_folderkey),
         [&](const api::folder::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 BOOST_CHECK(!response_data.folderkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFile)
{
    // Only .txt files allowed currently.
    globals::test_file_name = ut::RandomAlphaNum(20) + ".txt";

    Debug(globals::test_file_name);

    api::file::create::Request request;

    request.SetFilename(globals::test_file_name);
    request.SetParentFolderkey(globals::test_folderkey);

    Call(request, [&](const api::file::create::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 if (!response_data.links.normal_download)
                 {
                     Fail("Newly created file should have normal download "
                          "field for downloading file.");
                 }
                 else
                 {
                     Success();
                 }

                 globals::test_quickkey = response_data.quickkey;

                 BOOST_CHECK(!response_data.quickkey.empty());
                 BOOST_CHECK(response_data.created_datetime
                             != boost::date_time::not_a_date_time);
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CopyFile1)
{
    api::file::copy::Request request(globals::test_quickkey);

    request.SetTargetParentFolderkey(globals::test_folderkey);

    Call(request, [&](const api::file::copy::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::test_quickkey2 = response_data.quickkey;

                 Debug(response_data.quickkey);

                 BOOST_CHECK(!response_data.quickkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(UploadRandomFile)
{
    auto random_file = RandomFile(1024 * 1024 * 1);

    globals::upload::random_file_name = random_file.Name();

    mf::uploader::UploadManager um(&stm_);

    mf::uploader::UploadRequest request(globals::upload::random_file_name);

    request.SetTargetFolderkey(globals::test_folderkey);

    // You may want to change the name for a different test
    // request.SetTargetFilename(save_as);

    // Add the file to upload.
    um.Add(request, [this](mf::uploader::UploadStatus status)
           {
               if (auto error = boost::get<mf::uploader::upload_state::Error>(
                           &status.state))
               {
                   const auto & ec = error->error_code;
                   std::ostringstream ss;
                   ss << "Received error: " + ec.message() << "\n";
                   ss << "Error type: " + std::string(ec.category().name())
                      << "\n";
                   ss << "Error value: " + mf::utils::to_string(ec.value())
                      << "\n";
                   ss << "Description: " + error->description << "\n";

                   Fail(ss.str());
               }
               else if (auto success
                        = boost::get<mf::uploader::upload_state::Complete>(
                                &status.state))
               {
                   Log("New quickkey:", success->quickkey);
                   Log("New hash:", success->hash);

                   globals::upload::quickkey_1 = success->quickkey;
                   globals::upload::hash_1 = success->hash;

                   // New revisions available when new file created.
                   if (success->new_revision)
                   {
                       Log("First revision:", *success->new_revision);
                       globals::upload::upload_revision_1
                               = *success->new_revision;
                       Success();
                   }
                   else
                   {
                       Fail("New random file returned no new revision.");
                   }
               }
           });

    StartWithTimeout(posix_time::seconds(60));
}

BOOST_AUTO_TEST_CASE(VerifyOneVersionExists)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }

    api::file::get_versions::Request request(globals::upload::quickkey_1);

    Call(request, [&](const api::file::get_versions::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 // There should be at least two versions.
                 if (response_data.file_versions.size() != 1)
                 {
                     Fail("There should be only one file version after "
                          "replacement upload.");
                 }
                 else
                 {
                     Log("Got versions:");
                     for (auto & version : response_data.file_versions)
                         Log(version.revision);

                     if (response_data.file_versions[0].revision
                         != globals::upload::upload_revision_1)
                     {
                         Fail("The revision received from the uploader does "
                              "not match the revision from get_versions.");
                     }
                     else
                     {
                         Success();
                     }
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CopyFile2)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }

    api::file::copy::Request request(globals::upload::quickkey_1);

    request.SetTargetParentFolderkey(globals::test_folderkey);

    Call(request, [&](const api::file::copy::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::test_quickkey2 = response_data.quickkey;

                 Debug(response_data.quickkey);

                 BOOST_CHECK(!response_data.quickkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

// This should replace the file that was just uploaded, so we can restore it
// afterwards.
BOOST_AUTO_TEST_CASE(UploadFileReplacement)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }

    Debug("Uploading file.  This can take a few minutes depending on the "
          "server.");

    auto random_file = RandomFile(1024 * 1024 / 2 * 3);

    mf::uploader::UploadManager um(&stm_);

    mf::uploader::UploadRequest request(random_file.Name());

    request.SetTargetFolderkey(globals::test_folderkey);
    request.SetTargetFilename(globals::upload::random_file_name);
    request.SetOnDuplicateAction(mf::uploader::OnDuplicateAction::Replace);

    // You may want to change the name for a different test
    // request.SetTargetFilename(save_as);

    // Add the file to upload.
    um.Add(request, [this](mf::uploader::UploadStatus status)
           {
               if (auto error = boost::get<mf::uploader::upload_state::Error>(
                           &status.state))
               {
                   const auto & ec = error->error_code;
                   std::ostringstream ss;
                   ss << "Received error: " + ec.message() << "\n";
                   ss << "Error type: " + std::string(ec.category().name())
                      << "\n";
                   ss << "Error value: " + mf::utils::to_string(ec.value())
                      << "\n";
                   ss << "Description: " + error->description << "\n";

                   Fail(ss.str());
               }
               else if (auto success
                        = boost::get<mf::uploader::upload_state::Complete>(
                                &status.state))
               {
                   Log("Re-upload quickkey:", success->quickkey);
                   Log("New hash:", success->hash);

                   globals::upload::hash_2 = success->hash;

                   if (globals::upload::quickkey_1 != success->quickkey)
                   {
                       std::ostringstream ss;
                       ss << "Replacement upload has a different quickkey: "
                          << success->quickkey << " should be "
                          << globals::upload::quickkey_1;

                       Fail(ss.str());
                   }
                   else if (!success->new_revision)
                   {  // New revision should be available when new file created.
                       Fail("New random file returned no new revision. (2)");
                   }
                   else
                   {
                       Log("New revision:", *success->new_revision);
                       globals::upload::upload_revision_2
                               = *success->new_revision;

                       globals::upload::successful_replacement = true;

                       Success();
                   }
               }
           });

    StartWithTimeout(posix_time::minutes(5));
}

// The previous test should have created a new version in the cloud of that file
// that was replaced.
BOOST_AUTO_TEST_CASE(VerifyNewVersionExists)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }
    if (!globals::upload::successful_replacement)
    {
        Fail("Replacement was not successful.  Unable to proceed.");
        return;
    }

    Debug("Waiting for new revision to appear.  This can take a while.");

    api::file::get_versions::Request request(globals::upload::quickkey_1);

    using MakeRequest_t = std::function<void()>;

    MakeRequest_t MakeCall;

    // How long will we wait for the internal versioning system to catch up?
    // Accoring to one API person, 5 to 10 minutes
    const auto duration_till_timeout = skRevisionTimeout;

    const boost::posix_time::ptime timeout
            = boost::posix_time::second_clock::universal_time()
              + duration_till_timeout;

    MakeCall = [this, &request, timeout, &MakeCall]()
    {
        CallIn(boost::posix_time::seconds(2), request,
               [&](const api::file::get_versions::Response & response)
               {
                   const boost::posix_time::ptime now
                           = boost::posix_time::second_clock::universal_time();
                   const bool timed_out = now > timeout;
                   bool had_success = false;

                   if (!response.response_data)
                   {
                       Fail(response);
                   }
                   else
                   {
                       const auto & response_data = *response.response_data;

                       // There should be at least two versions.
                       if (response_data.file_versions.size() < 2)
                       {
                           if (timed_out)
                           {
                               Debug(response.debug);

                               Fail("There should be at least two file "
                                    "versions after replacement upload. "
                                    "Try running the long-wait version of "
                                    "this test if you are not doing so.");
                           }
                       }
                       else
                       {
                           bool original_revision_found = false;
                           bool new_revision_found = false;

                           for (auto & version_info :
                                response_data.file_versions)
                           {
                               if (version_info.revision
                                   == globals::upload::upload_revision_1)
                                   original_revision_found = true;
                               else if (version_info.revision
                                        == globals::upload::upload_revision_2)
                                   new_revision_found = true;
                           }

                           if (!original_revision_found)
                           {
                               if (timed_out)
                               {
                                   Debug(response.debug);
                                   Fail("Unable to find first upload revision "
                                        "in get_version.");
                               }
                           }
                           else if (!new_revision_found)
                           {
                               if (timed_out)
                               {
                                   Debug(response.debug);
                                   Fail("Unable to find second upload revision "
                                        "in get_version.");
                               }
                           }
                           else
                           {
                               Debug(response.debug);
                               Success();
                               had_success = true;
                               globals::multiple_revision_success = true;
                           }
                       }
                   }

                   // Retry if failure until we hit the timeout.
                   if (!had_success && !timed_out)
                   {
                       MakeCall();
                   }

               });

    };

    MakeCall();

    StartWithTimeout(duration_till_timeout + posix_time::seconds(15));
}

BOOST_AUTO_TEST_CASE(RestoreFile)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }
    if (!globals::upload::successful_replacement)
    {
        Fail("Replacement was not successful.  Unable to proceed.");
        return;
    }
    if (!globals::multiple_revision_success)
    {
        Fail("Failed getting original version.  Unable to proceed.");
        return;
    }

    api::file::restore::Request request(globals::upload::quickkey_1,
                                        globals::upload::upload_revision_1);

    Call(request, [&](const api::file::restore::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 std::cout << response.debug << std::endl;
                 globals::upload::file_restored = true;
                 Success();
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(VerifyRestoreFile)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }
    if (!globals::upload::successful_replacement)
    {
        Fail("Replacement was not successful.  Unable to proceed.");
        return;
    }

    if (globals::upload::file_restored)
    {
        Call(api::file::get_info::Request(globals::upload::quickkey_1),
             [&](const api::file::get_info::Response & response)
             {
                 // Success requires the following:
                 // 1. Result is "Success"
                 // 2. Hash is hash of requested version(original)
                 // 3. Revision is incremented
                 //    - Revisions must increment, otherwise device/get_changes
                 //      will not work properly.
                 if (!response.response_data)
                 {
                     Fail(response);
                 }
                 else
                 {
                     const auto & response_data = *response.response_data;

                     if (response_data.hash != globals::upload::hash_1)
                     {
                         Log("First hash:", globals::upload::hash_1);
                         Log("Second hash:", globals::upload::hash_2);
                         Log("Hash after restoration:", response_data.hash);
                         Log("Revisions. 1:",
                             globals::upload::upload_revision_1, "2:",
                             globals::upload::upload_revision_2, "Current:",
                             response_data.revision);

                         Fail("Hash on restored file incorrect!");
                     }
                     else if (response_data.revision
                              <= globals::upload::upload_revision_2)
                     {
                         Log("First hash:", globals::upload::hash_1);
                         Log("Second hash:", globals::upload::hash_2);
                         Log("Hash after restoration:", response_data.hash);
                         Log("Revisions. 1:",
                             globals::upload::upload_revision_1, "2:",
                             globals::upload::upload_revision_2, "Current:",
                             response_data.revision);

                         Fail("Revision on file not incremented!");
                     }
                     else
                     {
                         Success();
                     }
                 }
             });

        StartWithDefaultTimeout();
    }
}

BOOST_AUTO_TEST_CASE(GetFileVersions)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }
    if (!globals::upload::successful_replacement)
    {
        Fail("Replacement was not successful.  Unable to proceed.");
        return;
    }

    api::file::get_versions::Request request(globals::upload::quickkey_1);

    Call(request, [&](const api::file::get_versions::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 // There should be at least one version.
                 if (response_data.file_versions.size() > 0)
                 {
                     Success();
                 }
                 else
                 {
                     Fail("There should be at least one file version.");
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(RecentlyModified)
{
    if (!globals::upload::successful_replacement)
    {
        Fail("Replacement was not successful.  Unable to proceed.");
        return;
    }

    api::file::recently_modified::Request request;

    Call(request, [&](const api::file::recently_modified::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 // There should be at least one version.
                 if (response_data.quickkeys.size() > 0)
                 {
                     Success();
                 }
                 else
                 {
                     Fail("There should be at least one change.");
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(OneTimeKey)
{
    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }

    api::file::one_time_key::Request request;

    request.SetQuickkey(globals::upload::quickkey_1);

    Call(request, [&](const api::file::one_time_key::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 globals::one_time_key::download_token = response_data.token;
                 // There should be at least one version.
                 if (response_data.download_link)
                 {
                     Success();
                 }
                 else
                 {
                     Fail("Download link unavailable!");
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(ConfigureOneTimeKey)
{
    api::file::configure_one_time_key::Request request(
            globals::one_time_key::download_token);

    request.SetDurationMinutes(10);

    Call(request,
         [&](const api::file::configure_one_time_key::Response & response)
         {
             if (!response.response_data)
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

BOOST_AUTO_TEST_CASE(FileGetInfo)
{
    Call(api::file::get_info::Request(globals::test_quickkey2),
         [&](const api::file::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

#if 0  // Re-enable when #27247 is complete and we have upgraded to v1.3 of file/get_info

                if (!response_data.links.normal_download)
                {
                    Fail("All files should return normal download field for "
                         "downloading file.");
                }
                else
#endif
                 {
                     Success();
                 }

                 globals::test_file_name2 = response_data.filename;

                 Debug(globals::test_file_name2);

                 BOOST_CHECK(!response_data.quickkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileMove)
{
    // Move to root
    api::file::move::Request request(globals::test_quickkey2, "");

    Call(request, [&](const api::file::move::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileRename)
{
    api::file::update::Request request(globals::test_quickkey2);

    std::string new_name(ut::RandomAlphaNum(20));

    request.SetFilename(new_name);

    Call(request, [&](const api::file::update::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 Success();
                 globals::test_file_name2 = new_name;
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileMakePrivate)
{
    api::file::update::Request request(globals::test_quickkey2);

    request.SetPrivacy(api::file::update::Privacy::Private);

    Call(request, [&](const api::file::update::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileIsPrivate)
{
    Call(api::file::get_info::Request(globals::test_quickkey2),
         [&](const api::file::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 BOOST_CHECK(response_data.privacy
                             == api::file::get_info::Privacy::Private);
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeleteFile)
{
    if (globals::disable_deletes)
    {
        Debug("Skipping due to deletes being disabled.");
        return;
    }

    Call(api::file::file_delete::Request(globals::test_quickkey2),
         [&](const api::file::file_delete::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileGetLinks)
{
    std::vector<std::string> quickkeys = {globals::test_quickkey};
    Call(api::file::get_links::Request(quickkeys),
         [&](const api::file::get_links::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 if (response_data.links.empty())
                 {
                     Fail("No links were provided.");
                 }
                 else if (!response_data.links[0].one_time_download)
                 {
                     Fail("get_links should have provided a one time link.");
                 }
                 else
                 {
                     Success();
                 }
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(PreConfirmCopyMove)
{
    api::folder::get_info::Request request(globals::test_folderkey);

    request.SetDetails(api::folder::get_info::Details::FullDetails);

    Call(request, [&](const api::folder::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::copy_move::files = response_data.file_count;
                 globals::copy_move::folders = response_data.folder_count;
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CopyFolder)
{
    Call(api::folder::copy::Request(globals::test_folderkey,
                                    globals::test_folderkey2),
         [&](const api::folder::copy::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::test_folderkey3 = response_data.folderkey;
                 globals::test_folder_name3 = globals::test_folder_name;
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(MoveFolder)
{
    Call(api::folder::move::Request(globals::test_folderkey3,
                                    globals::test_folderkey),
         [&](const api::folder::move::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(RenameFolder)
{
    api::folder::update::Request request(globals::test_folderkey3);

    request.SetFoldername("CopyFolder");

    Call(request, [&](const api::folder::update::Response & response)
         {
             if (!response.response_data
                 || !response.response_data->device_revision)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();
                 globals::known_revision = *response_data.device_revision;
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(ConfirmCopyMove)
{
    api::folder::get_info::Request request(globals::test_folderkey);

    request.SetDetails(api::folder::get_info::Details::FullDetails);

    Call(request, [&](const api::folder::get_info::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 // std::cout << "files: " << response.total_files << std::endl;
                 // std::cout << "folders: " << response.total_folders <<
                 // std::endl;

                 BOOST_CHECK_EQUAL(response_data.file_count,
                                   globals::copy_move::files);
                 BOOST_CHECK_EQUAL(response_data.folder_count,
                                   globals::copy_move::folders + 1);
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(PurgeFile)
{
    std::vector<std::string> quickkeys = {globals::test_quickkey};
    Call(api::file::purge::Request(quickkeys),
         [&](const api::file::purge::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

// Create a second file for replacing the first one.
BOOST_AUTO_TEST_CASE(UploadRandomFile2)
{
    auto random_file = RandomFile(1024 * 1024 / 2 * 3);

    mf::uploader::UploadManager um(&stm_);

    mf::uploader::UploadRequest request(random_file.Name());

    request.SetTargetFolderkey(globals::test_folderkey);

    // You may want to change the name for a different test
    // request.SetTargetFilename(save_as);

    // Add the file to upload.
    um.Add(request, [this](mf::uploader::UploadStatus status)
           {
               if (auto error = boost::get<mf::uploader::upload_state::Error>(
                           &status.state))
               {
                   const auto & ec = error->error_code;
                   std::ostringstream ss;
                   ss << "Received error: " + ec.message() << "\n";
                   ss << "Error type: " + std::string(ec.category().name())
                      << "\n";
                   ss << "Error value: " + mf::utils::to_string(ec.value())
                      << "\n";
                   ss << "Description: " + error->description << "\n";

                   Fail(ss.str());
               }
               else if (auto success
                        = boost::get<mf::uploader::upload_state::Complete>(
                                &status.state))
               {
                   globals::upload::quickkey_2 = success->quickkey;
                   Success();
               }
           });

    StartWithTimeout(posix_time::seconds(60));
}

BOOST_AUTO_TEST_CASE(UpdateFile)
{
    return;

    if (globals::upload::quickkey_1.empty())
    {
        Fail("No quickkey available.");
        return;
    }
    if (globals::upload::quickkey_2.empty())
    {
        Fail("Second quickkey available.");
        return;
    }

    Call(api::file::update_file::Request(globals::upload::quickkey_2,
                                         globals::upload::quickkey_1),
         [&](const api::file::update_file::Response & response)
         {
             Debug(response.debug);

             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

#if 0  // API delete is asynchronous so next operations fail if we do this.
BOOST_AUTO_TEST_CASE(FolderDelete2)
{
    if (globals::disable_deletes)
    {
        Debug("Skipping due to deletes being disabled.");
        return;
    }

    Call(
        api::folder::folder_delete::Request(
            globals::test_folderkey2
            ),
        [&](const api::folder::folder_delete::Response & response)
        {
            if (!response.response_data)
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
#endif

BOOST_AUTO_TEST_CASE(DeviceGetChanges)
{
    uint32_t revision = 0;
    if (globals::known_revision > 500)
        revision = globals::known_revision - 500;
    revision -= revision % 500;

    std::ostringstream ss;
    ss << "Current revision: " << globals::known_revision;
    ss << " Checking revision: " << revision;
    Debug(ss.str());

    Call(api::device::get_changes::Request(revision),
         [&](const api::device::get_changes::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::has_updated_files
                         = !response_data.updated_files.empty();
                 globals::has_updated_folders
                         = !response_data.updated_folders.empty();
                 globals::has_deleted_files
                         = !response_data.deleted_files.empty();
                 globals::has_deleted_folders
                         = !response_data.deleted_folders.empty();
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeviceGetChanges2)
{
    uint32_t revision = 0;
    if (globals::known_revision > 500)
        revision = globals::known_revision - 500;
    revision += (500 - revision % 500);

    std::ostringstream ss;
    ss << "Current revision: " << globals::known_revision;
    ss << " Checking revision: " << revision;
    Debug(ss.str());

    Call(api::device::get_changes::Request(revision),
         [&](const api::device::get_changes::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::has_updated_files
                         |= !response_data.updated_files.empty();
                 globals::has_updated_folders
                         |= !response_data.updated_folders.empty();
                 globals::has_deleted_files
                         |= !response_data.deleted_files.empty();
                 globals::has_deleted_folders
                         |= !response_data.deleted_folders.empty();

                 // Files were created.
                 BOOST_CHECK(globals::has_updated_files);

                 // Folders were created.
                 BOOST_CHECK(globals::has_updated_folders);

                 // We have deleted files.
                 BOOST_CHECK(globals::has_deleted_files);

                 // We have deleted a folder.
                 // BOOST_CHECK( globals::has_deleted_folders );
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFolderUser2)
{
    SetUser2();

    globals::foreign_folder_name = ut::RandomAlphaNum(20);

    Debug(globals::foreign_folder_name);

    Call(api::folder::create::Request(globals::foreign_folder_name),
         [&](const api::folder::create::Response & response)
         {
             if (!response.response_data)
             {
                 Fail(response);
             }
             else
             {
                 const auto & response_data = *response.response_data;

                 Success();

                 globals::foreign_folderkey = response_data.folderkey;

                 BOOST_CHECK(!response_data.folderkey.empty());
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeviceGetStatusPreDelete1)
{
    Call(api::device::get_status::Request(),
         [&](const api::device::get_status::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderDelete1)
{
    if (globals::disable_deletes)
    {
        Debug("Skipping due to deletes being disabled.");
        return;
    }

    std::vector<std::string> folderkeys;

    std::function<void(const std::string &)> add_folderkey(
            [&](const std::string & folderkey)
            {
                if (!folderkey.empty())
                    folderkeys.push_back(folderkey);
            });
    add_folderkey(globals::test_folderkey);
    // add_folderkey(globals::test_folderkey2);
    // add_folderkey(globals::foreign_folderkey);

    if (!folderkeys.empty())
    {
        std::string keys(boost::join(folderkeys, ","));

        std::ostringstream ss;
        ss << "Deleting keys: " << keys;
        Debug(ss.str());

        Call(api::folder::folder_delete::Request(keys),
             [&](const api::folder::folder_delete::Response & response)
             {
                 if (!response.response_data)
                     Fail(response);
                 else
                     Success();
             });

        StartWithDefaultTimeout();
    }
}

BOOST_AUTO_TEST_CASE(DeviceGetStatusPreDelete2)
{
    Call(api::device::get_status::Request(),
         [&](const api::device::get_status::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderDelete2)
{
    if (globals::disable_deletes)
    {
        Debug("Skipping due to deletes being disabled.");
        return;
    }

    std::vector<std::string> folderkeys;

    std::function<void(const std::string &)> add_folderkey(
            [&](const std::string & folderkey)
            {
                if (!folderkey.empty())
                    folderkeys.push_back(folderkey);
            });
    // add_folderkey(globals::test_folderkey);
    add_folderkey(globals::test_folderkey2);
    // add_folderkey(globals::foreign_folderkey);

    if (!folderkeys.empty())
    {
        std::string keys(boost::join(folderkeys, ","));

        std::ostringstream ss;
        ss << "Deleting keys: " << keys;
        Debug(ss.str());

        Call(api::folder::folder_delete::Request(keys),
             [&](const api::folder::folder_delete::Response & response)
             {
                 if (!response.response_data)
                     Fail(response);
                 else
                     Success();
             });

        StartWithDefaultTimeout();
    }
}

BOOST_AUTO_TEST_CASE(DeviceGetStatusPostDelete)
{
    Call(api::device::get_status::Request(),
         [&](const api::device::get_status::Response & response)
         {
             if (!response.response_data)
                 Fail(response);
             else
                 Success();
         });

    StartWithDefaultTimeout();
}

/**
 * END UNIFIED OPERATIONS
 */

// Spam the server with the same call. Was used to verify claims made by
// another team that there could be a problem with session tokens expiring after
// 100 calls or so. No problem was discovered. Place assert in
// api::SessionMaintainer::HandleSessionTokenFailure to test again.
BOOST_AUTO_TEST_CASE(SpamTest)
{
    const uint32_t requests_to_be_made = 50;

    uint32_t times_failed = 0;
    uint32_t times_returned = 0;

    std::function<void(const api::device::get_changes::Response &, uint32_t)>
    callback([this, &times_returned, &times_failed, requests_to_be_made](
            const api::device::get_changes::Response & response, uint32_t count)
             {
                 std::ostringstream ss;
                 ss << "SpamTest: Count: " << count;
                 Debug(ss.str());

                 if (!response.response_data)
                 {
                     ++times_failed;
                     std::ostringstream error_ss;
                     error_ss << "Error: " << response.error_string;
                     Debug(error_ss.str());
                 }

                 ++times_returned;
                 if (times_returned == requests_to_be_made)
                 {
                     if (times_failed)
                         Fail(response);
                     else
                         Success();
                 }
             });

    for (uint32_t i = 1; i <= requests_to_be_made; ++i)
    {
        Call(api::device::get_changes::Request(0),
             std::bind(callback, std::placeholders::_1, i));
    }

    Start();

    BOOST_CHECK_EQUAL(times_failed, 0);
    BOOST_CHECK_EQUAL(times_returned, requests_to_be_made);
}

BOOST_AUTO_TEST_CASE(WARNING)
{
    if (globals::disable_deletes)
        Debug("Deletes are disabled!");
}
BOOST_AUTO_TEST_SUITE_END()
