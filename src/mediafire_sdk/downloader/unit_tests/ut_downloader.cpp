/**
 * @file mediafire_sdk/downloader/unit_tests/ut_downloader.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem/operations.hpp"

#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/error.hpp"

#include "mediafire_sdk/http/unit_tests/expect_server.hpp"
#include "mediafire_sdk/http/unit_tests/expect_server_ssl.hpp"

#include "mediafire_sdk/utils/base64.hpp"
#include "mediafire_sdk/utils/error.hpp"
#include "mediafire_sdk/utils/fileio.hpp"
#include "mediafire_sdk/utils/sha256_hasher.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/utils/variant.hpp"

#include "mediafire_sdk/downloader/detail/file_writer.hpp"
#include "mediafire_sdk/downloader/download.hpp"
#include "mediafire_sdk/downloader/download_status.hpp"
#include "mediafire_sdk/downloader/error.hpp"
#include "mediafire_sdk/downloader/reader/sha256_reader.hpp"
#include "mediafire_sdk/downloader/reader/md5_reader.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE UrlUnitTest
#include "boost/test/unit_test.hpp"

/** @todo hjones: Copy over expect example and modify it to build a
    downloadable file of zeros- 2015-06-24 */

namespace asio = boost::asio;
namespace config = mf::downloader::config;
namespace status = mf::downloader::status;

using mf::utils::Match;
using mf::utils::to_string;
using mf::utils::FileIO;

namespace constant
{
const std::string local_host = "127.0.0.1";
const uint16_t http_port = 8080;

}  // namespace constant

std::string GetHashOfFile(std::string path)
{
    const auto BUFFER_SIZE = int{1024};
    char buffer[BUFFER_SIZE];
    auto ec = std::error_code();
    auto hasher = mf::utils::Sha256Hasher();

    auto file_handle = FileIO::Open(path, "rb", &ec);
    if (!file_handle)
        return "";

    for (;;)
    {
        const auto bytes_read = file_handle->Read(buffer, BUFFER_SIZE, &ec);
        if (ec == mf::utils::file_io_error::EndOfFile)
        {
            return hasher.Digest();
        }
        else if (ec)
        {
            std::cout << __FUNCTION__ << ": Error: " << ec.message()
                      << std::endl;
            return "";
        }
        else
        {
            hasher.Update(bytes_read, buffer);
        }
    }
}

std::shared_ptr<asio::io_service::work> MakeWork(asio::io_service * io_service)
{
    return std::make_shared<asio::io_service::work>(*io_service);
}

std::shared_ptr<ExpectServer> MakeServer(asio::io_service * io_service)
{
    std::shared_ptr<ExpectServer> server = ExpectServer::Create(
            io_service, MakeWork(io_service), constant::http_port);

    // Set the action timeout. Should be a little more than the IO timeout we
    // set, so that we hit the IO timeout but not the action timeout, assuming
    // the IO timeout works.
    server->SetActionTimeoutMs(2000);

    return server;
}

std::string MakeUrl(uint16_t port, std::string query)
{
    std::string url = "http://";
    url += constant::local_host;
    url += ":";
    url += mf::utils::to_string(port);
    if (!query.empty())
    {
        url += "/";
        url += query;
    }
    return url;
}

class HeaderBuilder
{
public:
    HeaderBuilder & operator()(std::string line)
    {
        headers_ += line;
        headers_ += "\r\n";
        return *this;
    }

    std::string GetString() { return headers_ + "\r\n"; }

private:
    std::string headers_;
};

std::string Quote(std::string to_quote)
{
    std::string quoted("\"");
    for (auto ch : to_quote)
    {
        if (ch == '\\' || ch == '"')
            quoted += '\\';
        quoted += ch;
    }
    return quoted + "\"";
}

/**
 * @brief Generate a reproducible file that isn't too repetitive
 *
 * This creates a string that can be used as a file.  It will always generate
 * the same sequence with an orbit around 8589934580 bytes.  This ensures any
 * hash generated from the data will be consistent across runs of the program,
 * while ensuring some random offset isn't likely to generate the same sequence
 * as another.
 *
 * @param[in] start_byte Location in the "file" to start reading from.
 * @param[in] byte_length Number of bytes to return from the "file".
 *
 * @return String from the psuedo-random "file".
 */
std::string FileData(const uint64_t start_byte, const uint64_t byte_length)
{
    const auto p = uint64_t{4294967291UL};

    std::string result;
    result.reserve(byte_length);
    auto current_byte = start_byte;

    for (auto current_dword = current_byte / 4;; ++current_dword)
    {
        const auto unique_permutation = (current_dword * current_dword) % p;
        for (auto bid = current_byte % 4; bid < 4; ++bid, ++current_byte)
        {
            result += static_cast<uint8_t>((unique_permutation >> (8 * bid))
                                           & 0xFF);

            if (result.size() == byte_length)
                return result;
        }
    }
}

std::string ServerHeadersForDownload(std::string filename, uint64_t filesize)
{
    auto builder = HeaderBuilder();
    /* clang-format off */
    builder
        ("HTTP/1.1 200 OK")
        ("Server: LRBD-bigdownload-")
        ("Connection: close")
        ("Accept-Ranges: bytes")
        ("Content-transfer-encoding: binary")
        ("Content-Length: " + to_string(filesize))
        ("Content-Disposition: attachment; filename=" + Quote(filename))
        ("Content-Type: application/octet-stream")
        ;
    /* clang-format on */

    // std::cout << __FUNCTION__ << ": Headers being sent:\n" <<
    // builder.GetString() << std::endl;

    return builder.GetString();
}

std::string ServerHeadersForPartialDownload(std::string filename,
                                            uint64_t filesize,
                                            uint64_t start_byte)
{
    auto builder = HeaderBuilder();
    /* clang-format off */
    builder
        ("HTTP/1.1 206 Partial Content")
        ("Server: LRBD-bigdownload-")
        ("Connection: close")
        ("Accept-Ranges: bytes")
        ("Content-transfer-encoding: binary")
        ("Content-Length: " + to_string(filesize - start_byte))
        // Example: Content-Range: bytes 0-24101/24102
        ("Content-Range: bytes "
         + to_string(start_byte) + "-" + to_string(filesize-1)
         + "/" + to_string(filesize))
        ("Content-Disposition: attachment; filename=" + Quote(filename))
        ("Content-Type: application/octet-stream")
        ;
    /* clang-format on */

    // std::cout << __FUNCTION__ << ": Headers being sent:\n"
    //           << builder.GetString() << "END" << std::endl;

    return builder.GetString();
}

BOOST_AUTO_TEST_CASE(TestFileData)
{
    int start_pos = 0;
    int length = 10;
    int split_length = 5;

    BOOST_CHECK(FileData(start_pos, length)
                == FileData(start_pos, split_length)
                           + FileData(start_pos + split_length,
                                      length - split_length));

    start_pos = 1000001;
    length = 50;
    split_length = 23;

    BOOST_CHECK(FileData(start_pos, length)
                == FileData(start_pos, split_length)
                           + FileData(start_pos + split_length,
                                      length - split_length));

    BOOST_CHECK(FileData(0, 4) != FileData(8, 4));
}

BOOST_AUTO_TEST_CASE(DownloadToMemory)
{
    asio::io_service io_service;
    auto status_success = false;
    auto hash_from_memory = std::string{};

    auto status_callback = [&](mf::downloader::DownloadStatus new_status)
    {
        Match(new_status,
              [&](const status::Progress &)
              {
              },
              [&](const status::Failure & failure)
              {
                  std::cerr << "Failure: " << failure.description << "\n"
                            << "         (" << failure.error_code.message()
                            << "/" << failure.error_code.value() << ")"
                            << std::endl;

                  // Without calling this, any open sockets set to close will
                  // keep
                  // the io service running until the TCP connection is
                  // completely
                  // closed.
                  io_service.stop();
              },
              [&](const status::Success & success_data)
              {
                  Match(success_data.success_type,
                        [](const status::success::OnDisk &)
                        {
                            BOOST_FAIL("Wrong success type: OnDisk");
                        },
                        [&](const status::success::InMemory & in_memory)
                        {
                            const auto & buffer = in_memory.buffer;
                            auto hasher = mf::utils::Sha256Hasher();
                            std::for_each(std::begin(*buffer),
                                          std::end(*buffer),
                                          [&](char ch)
                                          {
                                              hasher.Update(1, &ch);
                                          });
                            hash_from_memory = hasher.Digest();

                            status_success = true;
                        },
                        [](const status::success::NoTarget &)
                        {
                            BOOST_FAIL("Wrong success type: NoTarget");
                        });

                  io_service.stop();
              });
    };

    auto server = MakeServer(&io_service);
    const auto filesize = uint64_t{1024 * 1024 * 5};

    server->Push(ExpectRegex{boost::regex(
            "GET.*\r\n"
            "\r\n")});

    // Expect a bunch of data.
    server->Push(expect_server_test::SendMessage(
            ServerHeadersForDownload("download_to_memory.bin", filesize)));

    const auto file_data_sha256 = std::string{
            [&]()
            { /* Encapsulate file_data to prevent accidental misuse as we are
                 moving it. */
              auto file_data = FileData(0, filesize);
              auto hash = mf::utils::HashSha256(file_data);
              server->Push(
                      expect_server_test::SendMessage(std::move(file_data)));
              return hash;
            }()};

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    using Writer = config::WriteToMemory;
    auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();

    auto download_configuration
            = mf::downloader::DownloadConfig(http_config, Writer{});
    download_configuration.AddReader(sha256_reader);

    mf::downloader::Download(MakeUrl(constant::http_port, "anyfile"),
                             download_configuration,
                             status_callback);

    try
    {
        io_service.run();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    BOOST_CHECK(status_success);
    BOOST_CHECK(server->Success());
    BOOST_CHECK(sha256_reader->GetHash() == file_data_sha256);
    BOOST_CHECK(hash_from_memory == file_data_sha256);
    BOOST_CHECK(!hash_from_memory.empty());
}

BOOST_AUTO_TEST_CASE(DownloadToFile)
{
    asio::io_service io_service;
    std::string filename("download_to_file.bin");
    boost::filesystem::remove(filename);
    bool status_success = false;

    auto status_callback = [&](mf::downloader::DownloadStatus new_status)
    {
        Match(new_status,
              [&](const status::Progress &)
              {
              },
              [&](const status::Failure & failure)
              {
                  std::cerr << "Failure: " << failure.description << "\n"
                            << "         (" << failure.error_code.message()
                            << "/" << failure.error_code.value() << ")"
                            << std::endl;

                  // Without calling this, any open sockets set to close will
                  // keep
                  // the io service running until the TCP connection is
                  // completely
                  // closed.
                  io_service.stop();
              },
              [&](const status::Success & success_data)
              {
                  Match(success_data.success_type,
                        [&](const status::success::OnDisk &)
                        {
                            status_success = true;
                        },
                        [](const status::success::InMemory &)
                        {
                            BOOST_FAIL("Wrong success type: InMemory");
                        },
                        [](const status::success::NoTarget &)
                        {
                            BOOST_FAIL("Wrong success type: NoTarget");
                        });

                  io_service.stop();
              });
    };

    auto server = MakeServer(&io_service);
    const auto filesize = uint64_t{1024 * 1024 * 5};

    server->Push(ExpectRegex{boost::regex(
            "GET.*\r\n"
            "\r\n")});

    // Expect a bunch of data.
    server->Push(expect_server_test::SendMessage(
            ServerHeadersForDownload(filename, filesize)));

    const auto file_data_sha256 = std::string{
            [&]()
            { /* Encapsulate file_data to prevent accidental misuse as we are
                 moving it. */
              auto file_data = FileData(0, filesize);
              auto hash = mf::utils::HashSha256(file_data);
              server->Push(
                      expect_server_test::SendMessage(std::move(file_data)));
              return hash;
            }()};

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    using Writer = config::WriteToFilesystemPath;
    using FileAction = Writer::FileAction;
    auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();

    auto download_configuration = mf::downloader::DownloadConfig(
            http_config, Writer{FileAction::FailIfExisting, filename});
    download_configuration.AddReader(sha256_reader);

    mf::downloader::Download(MakeUrl(constant::http_port, "anyfile"),
                             download_configuration,
                             status_callback);

    try
    {
        io_service.run();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    BOOST_CHECK(status_success);
    BOOST_CHECK(server->Success());
    BOOST_CHECK(sha256_reader->GetHash() == file_data_sha256);

    const auto hash_from_file = GetHashOfFile(filename);
    BOOST_CHECK(hash_from_file == file_data_sha256);
    BOOST_CHECK(!hash_from_file.empty());

    boost::system::error_code bec;
    const auto filesize_from_filesystem
            = boost::filesystem::file_size(filename, bec);
    BOOST_CHECK(filesize_from_filesystem == filesize);

    // std::cout << __FUNCTION__ << ": Hash of file: " << hash_from_file
    //           << std::endl;

    boost::filesystem::remove(filename);
}

BOOST_AUTO_TEST_CASE(DownloadToUnknownFile)
{
    asio::io_service io_service;
    std::string filename("download_to_unknown_file.bin");
    boost::filesystem::remove(filename);
    bool status_success = false;

    auto status_callback = [&](mf::downloader::DownloadStatus new_status)
    {
        Match(new_status,
              [&](const status::Progress &)
              {
              },
              [&](const status::Failure & failure)
              {
                  std::cerr << "Failure: " << failure.description << "\n"
                            << "         (" << failure.error_code.message()
                            << "/" << failure.error_code.value() << ")"
                            << std::endl;

                  // Without calling this, any open sockets set to close will
                  // keep
                  // the io service running until the TCP connection is
                  // completely
                  // closed.
                  io_service.stop();
              },
              [&](const status::Success & success_data)
              {
                  Match(success_data.success_type,
                        [&](const status::success::OnDisk &)
                        {
                            status_success = true;
                        },
                        [](const status::success::InMemory &)
                        {
                            BOOST_FAIL("Wrong success type: InMemory");
                        },
                        [](const status::success::NoTarget &)
                        {
                            BOOST_FAIL("Wrong success type: NoTarget");
                        });

                  io_service.stop();
              });
    };

    auto select_path_callback =
            [&](boost::optional<std::string> maybe_header_filename,
                const std::string & /*url*/,
                const mf::http::Headers & /*header_container*/)
                    -> mf::downloader::ErrorOrSaveLocation
    {
        if (maybe_header_filename)
        {
            return mf::downloader::ErrorOrSaveLocation::Value(
                    *maybe_header_filename);
        }
        else
        {
            return mf::downloader::ErrorOrSaveLocation::Error(
                    make_error_code(mf::downloader::errc::NoFilenameInHeader),
                    "No filename exists.");
        }
    };

    auto server = MakeServer(&io_service);
    const auto filesize = uint64_t{1024 * 1024 * 5};

    server->Push(ExpectRegex{boost::regex(
            "GET.*\r\n"
            "\r\n")});

    // Expect a bunch of data.
    server->Push(expect_server_test::SendMessage(
            ServerHeadersForDownload(filename, filesize)));

    const auto file_data_sha256 = std::string{
            [&]()
            { /* Encapsulate file_data to prevent accidental misuse as we are
                 moving it. */
              auto file_data = FileData(0, filesize);
              auto hash = mf::utils::HashSha256(file_data);
              server->Push(
                      expect_server_test::SendMessage(std::move(file_data)));
              return hash;
            }()};

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    using Writer = config::WriteToFilesystemUsingFilenameFromResponseHeader;
    auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();

    auto download_configuration = mf::downloader::DownloadConfig(
            http_config, Writer{select_path_callback});
    download_configuration.AddReader(sha256_reader);

    mf::downloader::Download(MakeUrl(constant::http_port, "anyfile"),
                             download_configuration,
                             status_callback);

    try
    {
        io_service.run();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    BOOST_CHECK(status_success);
    BOOST_CHECK(server->Success());
    BOOST_CHECK(sha256_reader->GetHash() == file_data_sha256);

    const auto hash_from_file = GetHashOfFile(filename);
    BOOST_CHECK(hash_from_file == file_data_sha256);
    BOOST_CHECK(!hash_from_file.empty());

    boost::system::error_code bec;
    const auto filesize_from_filesystem
            = boost::filesystem::file_size(filename, bec);
    BOOST_CHECK(filesize_from_filesystem == filesize);

    // std::cout << __FUNCTION__ << ": Hash of file: " << hash_from_file
    //           << std::endl;

    boost::filesystem::remove(filename);
}

namespace partial_test
{
const uint64_t filesize = uint64_t{1024 * 1024 * 5};
const std::string filename = "download_to_file2.bin";
}  // namespace partial_test

BOOST_AUTO_TEST_CASE(DownloadToFileIncomplete)
{
    /** This test downloads a file, but cancels it mid-download.  The
        incomplete download will be used in the next test as well. */

    asio::io_service io_service;
    boost::filesystem::remove(partial_test::filename);
    bool status_success = false;

    std::function<void()> cancel_download;

    auto status_callback = [&](mf::downloader::DownloadStatus new_status)
    {
        Match(new_status,
              [&](const status::Progress & progress)
              {
                  if (progress.bytes_read > (partial_test::filesize / 2))
                      cancel_download();
              },
              [&](const status::Failure & failure)
              {
                  if (failure.error_code == mf::downloader::errc::Cancelled)
                  {
                      status_success = true;
                  }
                  else
                  {
                      std::cerr << "Failure: " << failure.description << "\n"
                                << "         (" << failure.error_code.message()
                                << "/" << failure.error_code.value() << ")"
                                << std::endl;
                  }

                  // Without calling this, any open sockets set to close will
                  // keep the io service running until the TCP connection is
                  // completely closed.
                  io_service.stop();
              },
              [&](const status::Success & success_data)
              {
                  Match(success_data.success_type,
                        [&](const status::success::OnDisk &)
                        {
                            BOOST_FAIL("Wrong success type: OnDisk");
                        },
                        [](const status::success::InMemory &)
                        {
                            BOOST_FAIL("Wrong success type: InMemory");
                        },
                        [](const status::success::NoTarget &)
                        {
                            BOOST_FAIL("Wrong success type: NoTarget");
                        });

                  io_service.stop();
              });
    };

    auto server = MakeServer(&io_service);

    server->Push(ExpectRegex{boost::regex(
            "GET.*\r\n"
            "\r\n")});

    // Expect a bunch of data.
    server->Push(expect_server_test::SendMessage(ServerHeadersForDownload(
            partial_test::filename, partial_test::filesize)));

    server->Push(expect_server_test::SendMessage(
            FileData(0, partial_test::filesize)));

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    using Writer = config::WriteToFilesystemPath;
    using FileAction = Writer::FileAction;
    auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();

    auto download_configuration = mf::downloader::DownloadConfig(
            http_config,
            Writer{FileAction::FailIfExisting, partial_test::filename});
    download_configuration.AddReader(sha256_reader);

    auto download
            = mf::downloader::Download(MakeUrl(constant::http_port, "anyfile"),
                                       download_configuration,
                                       status_callback);

    cancel_download = [&]()
    {
        download->Cancel();
    };

    try
    {
        io_service.run();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    BOOST_CHECK(status_success);
}

BOOST_AUTO_TEST_CASE(DownloadToFileFromPartial)
{
    /** This test takes the previous incomplete file and does a partial download
        to complete the file. */

    asio::io_service io_service;
    bool status_success = false;

    boost::system::error_code bec;
    const auto partially_downloaded_filesize
            = boost::filesystem::file_size(partial_test::filename, bec);

    const auto file_data_sha256
            = mf::utils::HashSha256(FileData(0, partial_test::filesize));

    auto status_callback = [&](mf::downloader::DownloadStatus new_status)
    {
        Match(new_status,
              [&](const status::Progress &)
              {
              },
              [&](const status::Failure & failure)
              {
                  std::cerr << "Failure: " << failure.description << "\n"
                            << "         (" << failure.error_code.message()
                            << "/" << failure.error_code.value() << ")"
                            << std::endl;

                  // Without calling this, any open sockets set to close will
                  // keep the io service running until the TCP connection is
                  // completely closed.
                  io_service.stop();
              },
              [&](const status::Success & success_data)
              {
                  Match(success_data.success_type,
                        [&](const status::success::OnDisk &)
                        {
                            status_success = true;
                        },
                        [](const status::success::InMemory &)
                        {
                            BOOST_FAIL("Wrong success type: InMemory");
                        },
                        [](const status::success::NoTarget &)
                        {
                            BOOST_FAIL("Wrong success type: NoTarget");
                        });

                  io_service.stop();
              });
    };

    auto server = MakeServer(&io_service);

    server->SetActionTimeoutMs(1000 * 100);

    server->Push(ExpectRegex{boost::regex(
            "GET.*\r\n"
            ".*"
            "Range: bytes=0-"
            ".*"
            "\r\n\r\n")});

    server->Push(
            expect_server_test::SendMessage(ServerHeadersForPartialDownload(
                    partial_test::filename, partial_test::filesize, 0)));

    server->Push(ExpectDisconnectAndReconnect{});

    {
        std::string pattern;
        pattern +=
            "GET.*\r\n"
            ".*"
            "Range: bytes=" + to_string(partially_downloaded_filesize) +  "-"
            + to_string(partial_test::filesize) +
            ".*"
            "\r\n\r\n";

        server->Push(ExpectRegex{boost::regex(pattern.data())});
    }

    server->Push(expect_server_test::SendMessage(
            ServerHeadersForPartialDownload(partial_test::filename,
                                            partial_test::filesize,
                                            partially_downloaded_filesize)));

    // Server will provide only the requested data.
    server->Push(expect_server_test::SendMessage(
            FileData(partially_downloaded_filesize,
                     partial_test::filesize - partially_downloaded_filesize)));

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    using Writer = config::ContinueWritingToFilesystemPath;
    auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();

    auto download_configuration = mf::downloader::DownloadConfig(
            http_config, Writer{partial_test::filename, boost::none});

    download_configuration.AddReader(sha256_reader);

    mf::downloader::Download(MakeUrl(constant::http_port, "anyfile"),
                             download_configuration,
                             status_callback);

    try
    {
        io_service.run();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    BOOST_CHECK(status_success);
    BOOST_CHECK(server->Success());
    BOOST_CHECK(sha256_reader->GetHash() == file_data_sha256);

    const auto hash_from_file = GetHashOfFile(partial_test::filename);
    BOOST_CHECK(hash_from_file == file_data_sha256);
    BOOST_CHECK(!hash_from_file.empty());

    const auto filesize_from_filesystem
            = boost::filesystem::file_size(partial_test::filename, bec);
    BOOST_CHECK(filesize_from_filesystem == partial_test::filesize);

    // std::cout << __FUNCTION__ << ": Hash of file: " << hash_from_file
    //           << std::endl;

    // boost::filesystem::remove(partial_test::filename);
}
