# Introduction

The MediaFire C++ API SDK simplifies communicating with the MediaFire API.

Features:
* Manage the user session
* Manage uploads to MediaFire
* Communicate with the MediaFire API

# Requirements

## C++

This library uses the C++11 version of C++.

## Libraries

[Boost](http://www.boost.org/) is required to build this library.

MinGW users, please read MINGW.NOTES

## Build System

This library requires [cmake](http://www.cmake.org/) in order to be built.

# Project Integration

In your project, you need to provide the following. Example code is provided in the Examples directory.

## Required Variables

### CMake variables

A few variables should be defined in your cmake cache in order to run all the unit tests.

* TEST_USER_1_USERNAME
  * Username to first test account.
* TEST_USER_1_PASSWORD
  * Password to first test account.
* TEST_USER_2_USERNAME
  * Username to second test account.
* TEST_USER_2_PASSWORD
  * Password to second test account.

#### Example config

```cmake
SET(TEST_USER_1_USERNAME "test@test.com" CACHE STRING "User 1 username")
SET(TEST_USER_1_PASSWORD "password" CACHE STRING "User 1 password")
SET(TEST_USER_2_USERNAME "test2@test.com" CACHE STRING "User 2 username")
SET(TEST_USER_2_PASSWORD "password" CACHE STRING "User 2 password")
```

## Optional Variables

### CMake variables

* MFAPI_APP_CONSTANTS_LIBRARY
  * If supplying your own kAppId or BuildSignature, you will need to pass the library name to link against in the SDK.
* MFAPI_APP_ID
  * Convenience variable for MFAPI_APP_ID define.
* MFAPI_APP_KEY
  * Convenience variable for MFAPI_APP_KEY define.

### Defines

* MFAPI_OVERRIDE_APP_ID
  * Define if supplying your own app_constants::kAppId.
* MFAPI_OVERRIDE_BUILD_SIGNATURE
  * Define if supplying your own app_constants::BuildSignature.
* MFAPI_APP_ID
  * Set the application ID.
* MFAPI_APP_KEY
  * Set the application key.

### Example

```cmake
# We supply the app id variable app_constants::kAppId
add_definitions(-DMFAPI_OVERRIDE_APP_ID)

# We supply the function app_constants::BuildSignature
add_definitions(-DMFAPI_OVERRIDE_BUILD_SIGNATURE)

# We supply those functions in a library we name our_app_constants
set(MFAPI_APP_CONSTANTS_LIBRARY our_app_constants)

# We don't use the internal app_constants::kAppId in the SDK
#add_definitions(-DMFAPI_APP_ID=My_app_id)

# We don't supply an application key for use in app_constants::BuildSignature
# inside the SDK.
#add_definitions(-DMFAPI_APP_KEY=My_app_key)

include_directories("api_sdk/src")
add_subdirectory("api_sdk/src" "${PROJECT_BINARY_DIR}/api_sdk")
```

# Example code

The library is composed of a few major components, the HTTP module which makes HTTP requests in behalf of the user, and the API module which manages session and connection for the user and uses the HTTP module to make its requests.  There is also an upload manager using uses the other modules to upload files.  Normally you will only need to use the API and uploader modules.

## API Module

The majority of API requests require a session token.  In order to prevent the user from having to manage session tokens, the <tt>SessionMaintainer</tt> handles all API requests instead of the user having to make direct <tt>HttpRequest</tt>s.  The SessionMaintainer can also manage API requests that do not require a session token, in order to keep track of errors and connection status.

### API Requests

APIs follow a pattern.  Each API has a <tt>Request</tt> class and a <tt>Response</tt> class.  APIs are namespaced so that the classes and associated types do not conflict and are organized nicely.

The callback passed to <tt>SessionMaintainer::Call</tt> when taking a <tt>api::user::get_info::Request</tt> will have the matching signature "<tt>void (const api::user::get_info::Response &)</tt>".

### Basic Usage

A few includes:

```cpp
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/user/get_info.hpp"
```

Create the <tt>SessionMaintainer</tt>:

```cpp
asio::io_service io_service;
auto http_config = mf::http::HttpConfig::Create(&io_service);
api::SessionMaintainer stm(&http_config);
```

Set user:

```cpp
stm.SetLoginCredentials( api::credentials::Email{ username, password } );
```

Make a request:

```cpp
api::user::get_info::Request request;
stm->Call(
    request,
    [&](const api::user::get_info::Response & response)
    {
        if ( response.error_code )
        {
            std::cout << response.error_string << std::endl;
        }
        else
        {
            std::cout << "Success.\nDisplay name: " << response.display_name
                << std::endl;
        }
        io_service.stop();
    });

// Start the io_service, which will run until the call is completed and stop()
// is called.
io_service.run();
```

## Downloader

Includes:

```cpp
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/downloader/detail/file_writer.hpp"
#include "mediafire_sdk/downloader/download.hpp"
#include "mediafire_sdk/downloader/download_status.hpp"
#include "mediafire_sdk/downloader/error.hpp"
#include "mediafire_sdk/downloader/reader/sha256_reader.hpp"
#include "mediafire_sdk/downloader/reader/md5_reader.hpp"
#include "mediafire_sdk/http/url.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/utils/variant.hpp"
```

Create a status handler.

```cpp
void StatusHandler(mf::downloader::DownloadStatus new_status)
{
    namespace status = mf::downloader::status;

    Match(new_status,
          [](const status::Progress &)
          {
          },
          [](const status::Failure & failure)
          {
              std::cerr << "Failure: " << failure.description << "\n"
                        << "         (" << failure.error_code.message() << "/"
                        << failure.error_code.value() << ")" << std::endl;
          },
          [](const status::Success & success_data)
          {
              // Or if you are certain of the success type, use boost::get.
              Match(success_data.success_type,
                    [](const status::success::OnDisk & on_disk)
                    {
                        std::cerr << "Filename: " << on_disk.filepath
                                  << std::endl;
                    },
                    [](const status::success::InMemory & in_memory)
                    {
                        const auto & buffer = in_memory.buffer;
                        std::copy(std::begin(*buffer),
                                  std::end(*buffer),
                                  std::ostream_iterator<char>(std::cout));
                        std::cout << std::endl;
                    },
                    [](const status::success::NoTarget &)
                    {
                    });
          });
}
```

Create the http module.

```cpp
asio::io_service io_service;

auto http_config = mf::http::HttpConfig::Create();
http_config->SetWorkIoService(&io_service);
```

Configure the download.

```cpp
mf::downloader::StatusCallback status_callback = boost::bind(&StatusHandler, _1);

using Writer = config::WriteToFilesystemPath;
using FileAction = Writer::FileAction;

auto download_configuration = mf::downloader::DownloadConfig(
        http_config,
        Writer{FileAction::FailIfExisting, download_path});
```

Add optional readers, such as one that calculates the SHA256 of the file as it downloads.

```cpp
auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();
download_configuration.AddReader(sha256_reader);
```

Create the download.

```cpp
mf::downloader::Download(
        url_str, download_configuration, status_callback);
```

Start the io service which performs all operations.

```cpp
io_service.run();
```

Now wait for the callback to be called with the result of the download.

## Uploader

Includes:

```cpp
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/uploader/upload_manager.hpp"
```

Create the connection and upload managers.

```cpp
asio::io_service io_service;

auto http_config = mf::http::HttpConfig::Create();
http_config->SetWorkIoService(&io_service);

mf::api::SessionMaintainer stm(http_config);

// Handle session token failures.
stm.SetSessionStateChangeCallback(
    [&io_service](mf::api::SessionState state)
    {
        if (boost::get<mf::api::session_state::CredentialsFailure>(&state))
        {
            std::cout << "Username or password incorrect."
            << std::endl;
            io_service.stop();
        }
    });

stm.SetLoginCredentials( mf::api::credentials::Email{ username, password } );

mf::uploader::UploadManager upload_manager(&stm);
```

Make an upload request.

```cpp
mf::uploader::UploadRequest request(upload_file_path);

if (vm.count("folderkey"))
    request.SetTargetFolderkey(folderkey);

if (vm.count("path"))
    request.SetTargetFolderPath(directory_path);

if (vm.count("saveas"))
    request.SetTargetFilename(directory_path);

if (vm.count("replace"))
{
    request.SetOnDuplicateAction(
        mf::uploader::OnDuplicateAction::Replace);
}

upload_manager.Add(request,
    [&io_service](mf::uploader::UploadStatus status)
    {
        if (auto * err_state = boost::get<us::Error>(&status.state))
        {
            std::cout << "Received error: " << err_state->error_code.message()
                << std::endl;
            std::cout << "Description: " << err_state->description << std::endl;
            io_service.stop();
        }
        else if (auto * complete_state = boost::get<us::Complete>(&status.state))
        {
            std::cout << "Upload complete.\nNew quickkey: " <<
                complete_state->quickkey << std::endl;
            io_service.stop();
        }
    });
```

And don't forget to start the io_service, which can run as your main loop or in a separate thread.

```cpp
io_service.run();
```

## HTTP Module

Unlike <tt>SessionMaintainer</tt> API requests, <tt>HttpRequest</tt>s have no maintainer.  Each request is performed by an io_service then the result is passed on via the <tt>RequestResponseInterface</tt>.

### Basic Usage

This module allows simple HTTP requests to be made.

A few includes:

```cpp
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include "mediafire_sdk/http/http_request.hpp"
```

Create a class to handle the response:

```cpp
class ResponseHandler : public RequestResponseInterface
{
public:
    /**
     * @brief Called after response header is parsed with redirect directions
     * which are being followed.
     *
     * @param[in] raw_header The headers in plain text.
     * @param[in] headers Headers parsed into parts.
     * @param[in] new_url New request target.
     */
    virtual void RedirectHeaderReceived(
            const mf::http::Headers & headers,
            const Url & new_url
        )
    {
        std::cout << "Got redirected" << std::endl;
    }

    /**
     * @brief Called after response header is parsed.
     *
     * @param[in] headers Headers parsed into parts.
     */
    virtual void ResponseHeaderReceived(
            const Headers & headers
        )
    {
        std::cout << "Got headers: " << headers.raw_headers << std::endl;
    }

    /**
     * @brief Called when content received.
     *
     * As content is streamed from the remote server, this is called with the
     * streamed content.
     *
     * @param[in] start_pos Where in the response content the buffer starts.
     * @param[in] buffer The streamed data from the remote server.
     */
    virtual void ResponseContentReceived(
            size_t start_pos,
            std::shared_ptr<BufferInterface> buffer
        )
    {
        std::cout << "Got content:\n"
            << std::string(buffer->Data(), buffer->Size()) << std::endl;
    }

    /**
     * @brief Called when an error occurs. Completes the request.
     *
     * @param[in] error_code The error code of the error.
     * @param[in] error_text Long description of the error.
     */
    virtual void RequestResponseErrorEvent(
            std::error_code error_code,
            std::string error_text
        )
    {
        std::cout << "Got error: " << error_code.message() << ": " << error_text
            << std::endl;

        std::cout << "Request complete due to error." << std::endl;
    }

    /**
     * @brief Called when the request is successful. Completes the request.
     */
    virtual void RequestResponseCompleteEvent() = 0;
    {
        std::cout << "Request complete due to success." << std::endl;
    }
};
```

Perform a request:

```cpp
asio::io_service io_service;

std::shared_ptr<ResponseHandler> rh = std::make_shared<ResponseHandler>();

auto http_config = mf::http::HttpConfig::Create(&io_service);
auto request = mf::http::HttpRequest::Create(
    http_config,
    std::static_pointer_cast<
        mf::http::RequestResponseInterface>(rh),
    &io_service,
    "http://www.mediafire.com");

// Start the request.
request->Start();

// run() will stop when it has no more work to do, i.e. when the request is
// complete.
io_service.run();
```
