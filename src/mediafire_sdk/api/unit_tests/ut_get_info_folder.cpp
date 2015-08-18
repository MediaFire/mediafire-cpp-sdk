#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/get_info_folder.hpp"

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

BOOST_AUTO_TEST_CASE(UTGetInfoFolder)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using GetInfoFolderType
            = mf::api::GetInfoFolder<mf::api::folder::get_info::Request>;

    using RequestType = GetInfoFolderType::RequestType;
    using ResponseType = GetInfoFolderType::ResponseType;

    using GetInfoFolderErrorType = typename GetInfoFolderType::ErrorType;

    GetInfoFolderType::CallbackType HandleGetInfoFolder =
            [this, &io_service](const ResponseType & response,
                                const std::vector<GetInfoFolderErrorType> &
                                        errors)  // TODO: Add errors
    {
        if (response.response_data)
            std::cout << response.response_data->folderkey << std::endl;
        else
            std::cout << "No folderkey" << std::endl;

        io_service.stop();
    };

    auto get_folder_info = GetInfoFolderType::Create(
            &stm, "", std::move(HandleGetInfoFolder));

    get_folder_info->Start();

    io_service.run();
}
