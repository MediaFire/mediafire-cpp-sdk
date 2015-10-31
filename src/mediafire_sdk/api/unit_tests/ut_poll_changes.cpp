#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/poll_changes.hpp"

#include "mediafire_sdk/api/device/get_status.hpp"
#include "mediafire_sdk/api/device/get_changes.hpp"
#include "mediafire_sdk/api/file/get_info.hpp"
#include "mediafire_sdk/api/folder/get_info.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE PollChanges
#include "boost/test/unit_test.hpp"

#if !defined(TEST_USER_1_USERNAME) || !defined(TEST_USER_1_PASSWORD)
#error "TEST_USER defines not set."
#endif

namespace
{
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
}  // end anonymous namespace

namespace api = mf::api;

BOOST_AUTO_TEST_CASE(UTPollChanges)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using DeviceGetStatusType = mf::api::device::get_status::Request;
    using DeviceGetChangesType = mf::api::device::get_changes::Request;
    using FolderGetInfoType = mf::api::folder::get_info::Request;
    using FileGetInfoType = mf::api::file::get_info::Request;

    using PollChangesType
            = mf::api::PollChanges<DeviceGetStatusType, DeviceGetChangesType,
                                   FolderGetInfoType, FileGetInfoType>;

    // Response types
    using File = PollChangesType::File;
    using Folder = PollChangesType::Folder;
    using FolderInfo = PollChangesType::FolderInfo;
    using FileInfo = PollChangesType::FileInfo;

    // Error types
    using DeviceGetStatusErrorType =
            PollChangesType::DeviceGetStatusErrorType;
    using DeviceGetChangesErrorType =
            PollChangesType::DeviceGetChangesErrorType;
    using GetInfoFileErrorType = PollChangesType::FileGetInfoErrorType;
    using GetInfoFolderErrorType =
            PollChangesType::FolderGetInfoErrorType;

    auto HandlePollChanges = [this, &io_service](
            uint32_t latest_device_revision,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<FileInfo> & updated_files_info,
            const std::vector<FolderInfo> & updated_folders_info,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<DeviceGetChangesErrorType> & get_changes_errors,
            const std::vector<GetInfoFileErrorType> & get_info_file_errors,
            const std::vector<GetInfoFolderErrorType> &
                    get_info_folder_errors)  // TODO: Add errors
    {
        if (!get_status_errors.empty())
            BOOST_FAIL("device/get_status returned errors");

        if (!get_changes_errors.empty())
            BOOST_FAIL("device/get_changes returned errors");

        if (!get_info_file_errors.empty())
            BOOST_FAIL("file/get_info returned errors");

        if (!get_info_folder_errors.empty())
            BOOST_FAIL("folder/get_info returned errors");

        for (const auto & updated_file_info : updated_files_info)
        {
            if (updated_file_info.response_data)
            {
                const auto & data = *updated_file_info.response_data;
                std::cout << "Updated File Info: " << data.quickkey
                          << std::endl;
            }
            else
            {
                std::cout << "Updated File Info: Missing data!" << std::endl;
            }
        }

        for (const auto & updated_folder_info : updated_folders_info)
        {
            if (updated_folder_info.response_data)
            {
                const auto & data = *updated_folder_info.response_data;
                std::cout << "Updated Folder Info: " << data.folderkey
                          << std::endl;
            }
            else
            {
                std::cout << "Updated Folder Info: Missing data!" << std::endl;
            }
        }

        for (const auto & deleted_file : deleted_files)
        {
            std::cout << "Deleted File: " << deleted_file.quickkey << std::endl;
        }

        for (const auto & deleted_folder : deleted_folders)
        {
            std::cout << "Deleted Folder: " << deleted_folder.folderkey
                      << std::endl;
        }

        io_service.stop();
    };

    auto work_manager = mf::api::WorkManager::Create(&io_service);

    auto poll_changes = PollChangesType::Create(&stm, 0, work_manager,
                                                std::move(HandlePollChanges));
    poll_changes->Start();

    io_service.run();
}
