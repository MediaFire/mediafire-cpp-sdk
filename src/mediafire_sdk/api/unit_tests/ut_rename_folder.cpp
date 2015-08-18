#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/rename_folder.hpp"

#include "mediafire_sdk/api/folder/update.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE RenameFolder
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

BOOST_AUTO_TEST_CASE(UTRenameFolder)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using RenameFolderType = mf::api::RenameFolder<mf::api::folder::update::Request>;

    using RequestType = RenameFolderType::RequestType;
    using ResponseType = RenameFolderType::ResponseType;

    using RenameFolderErrorType = typename RenameFolderType::ErrorType;

    RenameFolderType::CallbackType HandleRenameFolder =
    [this, &io_service](const ResponseType & response,
                        const std::vector<RenameFolderErrorType> &
                        errors)  // TODO: Add errors
    {
        io_service.stop();
    };

    auto rename_folder
    = RenameFolderType::Create(&stm, "", "", std::move(HandleRenameFolder));

    // Just make sure it compiles

    //    rename_folder->Start();
    //
    //    io_service.run();
}
