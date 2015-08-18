#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/rename_file.hpp"

#include "mediafire_sdk/api/file/update.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE RenameFile
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

BOOST_AUTO_TEST_CASE(UTRenameFile)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using RenameFileType = mf::api::RenameFile<mf::api::file::update::Request>;

    using RequestType = RenameFileType::RequestType;
    using ResponseType = RenameFileType::ResponseType;

    using RenameFileErrorType = typename RenameFileType::ErrorType;

    RenameFileType::CallbackType HandleRenameFile =
            [this, &io_service](const ResponseType & response,
                                const std::vector<RenameFileErrorType> &
                                        errors)  // TODO: Add errors
    {
        io_service.stop();
    };

    auto rename_file
            = RenameFileType::Create(&stm, "", "", std::move(HandleRenameFile));

    // Just make sure it compiles

    //    rename_file->Start();
    //
    //    io_service.run();
}
