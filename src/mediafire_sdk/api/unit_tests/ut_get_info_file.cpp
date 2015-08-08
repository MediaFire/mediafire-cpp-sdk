#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/get_info_file.hpp"

#include "mediafire_sdk/api/file/get_info.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE GetInfoFile
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

BOOST_AUTO_TEST_CASE(UTGetInfoFile)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using GetInfoFileType
            = mf::api::GetInfoFile<mf::api::file::get_info::Request>;

    using RequestType = GetInfoFileType::RequestType;
    using ResponseType = GetInfoFileType::ResponseType;

    GetInfoFileType::CallbackType HandleGetInfoFile = [this, &io_service](
            const ResponseType & response,
            const std::vector<GetInfoFileType::ErrorType> & errors)
    {
        if (!errors.empty())
        {
            BOOST_FAIL("HandleGetInfoFile reported errors!");
        }
        else if (!response.response_data)
        {
            BOOST_FAIL("HandleGetInfoFile missing response_data!");
        }
        else
        {
            std::cout << response.response_data->quickkey << std::endl;
        }

        io_service.stop();
    };

    auto get_folder_info = GetInfoFileType::Create(
            &stm, "",
            std::move(HandleGetInfoFile));  // TODO: Add a default quickkey

    get_folder_info->Start();

    io_service.run();
}
