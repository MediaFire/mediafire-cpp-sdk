/**
 * @file mediafire_sdk/downloader/unit_tests/download_url.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "boost/bind.hpp"
#include "boost/exception/diagnostic_information.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/program_options.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#include "mediafire_sdk/downloader/detail/file_writer.hpp"
#include "mediafire_sdk/downloader/download.hpp"
#include "mediafire_sdk/downloader/download_status.hpp"
#include "mediafire_sdk/downloader/error.hpp"
#include "mediafire_sdk/downloader/reader/sha256_reader.hpp"
#include "mediafire_sdk/downloader/reader/md5_reader.hpp"
#include "mediafire_sdk/http/url.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/utils/variant.hpp"

namespace po = boost::program_options;
namespace asio = boost::asio;
namespace config = mf::downloader::config;

using mf::utils::Match;
using Map = std::map<std::string, int>;

void ShowUsage(const char * filename, const po::options_description & visible)
{
    std::cerr << "Usage: " << filename << " [options]"
                                          " URLS\n";
    std::cerr << visible << "\n";
}

bool Contains(const std::map<std::string, int> & container,
              const std::string & value)
{
    return (std::find_if(std::begin(container),
                         std::end(container),
                         [&](const Map::value_type & pair)
                         {
                             return pair.first == value;
                         }) != std::end(container));
}

bool PathExists(std::string filepath)
{
    return boost::filesystem::exists(filepath);
}

class SelectUniqueFilePathCallbackHandler
{
public:
    SelectUniqueFilePathCallbackHandler() {}

    mf::downloader::ErrorOrSaveLocation operator()(
            boost::optional<std::string> maybe_header_filename,
            const std::string & /*url*/,
            const mf::http::Headers & /*header_container*/)
    {
        if (maybe_header_filename)
        {
            auto filename = *maybe_header_filename;

            if (PathExists(filename))
            {
                auto message = ("Filename already exists: ") + filename;
                return mf::downloader::ErrorOrSaveLocation::Error(
                        make_error_code(mf::downloader::errc::OverwriteDenied),
                        message);
            }
            else
            {
                return mf::downloader::ErrorOrSaveLocation::Value(
                        std::move(filename));
            }
        }
        else
        {
            return mf::downloader::ErrorOrSaveLocation::Error(
                    make_error_code(mf::downloader::errc::NoFilenameInHeader),
                    "No filename exists.");
        }
    }

private:
};

class SelectFilePathCallbackHandler
{
public:
    SelectFilePathCallbackHandler() {}

    mf::downloader::ErrorOrSaveLocation operator()(
            boost::optional<std::string> maybe_header_filename,
            const std::string & /*url*/,
            const mf::http::Headers & /*header_container*/)
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
    }

private:
};

void StatusHandler(asio::io_service * io_service,
                   const std::string & /*url_str*/,
                   const std::string & /*filename*/,
                   bool & success,
                   mf::downloader::DownloadStatus new_status)
{
    namespace status = mf::downloader::status;

    static bool wrote_dots = false;

    Match(new_status,
          [&](const status::Progress &)
          {
              std::cerr << ".";
              wrote_dots = true;
          },
          [&](const status::Failure & failure)
          {
              if (wrote_dots)
                  std::cerr << "\n";
              std::cerr << "Failure: " << failure.description << "\n"
                        << "         (" << failure.error_code.message() << "/"
                        << failure.error_code.value() << ")" << std::endl;

              // Without calling this, any open sockets set to close will keep
              // the io service running until the TCP connection is completely
              // closed.
              io_service->stop();
          },
          [&](const status::Success & success_data)
          {
              if (wrote_dots)
                  std::cerr << "\n";
              success = true;

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

int main(int argc, char * argv[])
{
    try
    {
        std::string url_str;
        std::string download_path;
        bool continue_download = false;

        po::options_description visible("Allowed options");
        /* clang-format off */
        visible.add_options()
            ("help,h"     ,                                         "Show this message.")
            ("continue,c" ,                                         "Continue previous download.")
            ("output,o"   , po::value<std::string>(&download_path), "Output file")
            ;
        /* clang-format on */

        po::options_description hidden("Hidden options");
        hidden.add_options()("url", po::value<std::string>(&url_str), "url");

        po::positional_options_description p;
        p.add("url", 1);

        po::options_description cmdline_options;
        cmdline_options.add(visible).add(hidden);

        po::variables_map vm;

        try
        {
            po::store(po::command_line_parser(argc, argv)
                              .options(cmdline_options)
                              .positional(p)
                              .run(),
                      vm);
            po::notify(vm);
        }
        catch (boost::program_options::error & err)
        {
            ShowUsage(argv[0], visible);
            return 1;
        }

        if (vm.count("help"))
        {
            ShowUsage(argv[0], visible);
            return 0;
        }

        if (vm.count("continue"))
        {
            continue_download = true;
        }

        if (!vm.count("url"))
        {
            ShowUsage(argv[0], visible);
            return 1;
        }

        asio::io_service io_service;

        auto http_config = mf::http::HttpConfig::Create();
        http_config->SetWorkIoService(&io_service);

        auto sha256_reader = std::make_shared<mf::downloader::Sha256Reader>();
        auto md5_reader = std::make_shared<mf::downloader::Md5Reader>();
        bool success = false;

        try
        {
            mf::downloader::StatusCallback status_callback
                    = boost::bind(&StatusHandler,
                                  &io_service,
                                  url_str,
                                  download_path,
                                  std::ref(success),
                                  _1);

            if (continue_download)
            {
                using Writer = config::ContinueWritingToFilesystemPath;

                auto download_configuration = [&download_path, &http_config]()
                {
                    if (download_path.empty())
                    {
                        return mf::downloader::DownloadConfig(
                                http_config,
                                Writer{SelectFilePathCallbackHandler(),
                                       boost::none});
                    }
                    else
                    {
                        return mf::downloader::DownloadConfig(
                                http_config,
                                Writer{download_path, boost::none});
                    }
                }();

                download_configuration.AddReader(sha256_reader);
                download_configuration.AddReader(md5_reader);

                mf::downloader::Download(
                        url_str, download_configuration, status_callback);
            }
            else if (download_path.empty())
            {
                using Writer = config::
                        WriteToFilesystemUsingFilenameFromResponseHeader;

                auto path_select_callback
                        = SelectUniqueFilePathCallbackHandler();

                auto download_configuration = mf::downloader::DownloadConfig(
                        http_config, Writer{path_select_callback});
                download_configuration.AddReader(sha256_reader);
                download_configuration.AddReader(md5_reader);

                mf::downloader::Download(
                        url_str, download_configuration, status_callback);
            }
            else
            {
                using Writer = config::WriteToFilesystemPath;
                using FileAction = Writer::FileAction;

                auto download_configuration = mf::downloader::DownloadConfig(
                        http_config,
                        Writer{FileAction::FailIfExisting, download_path});
                download_configuration.AddReader(sha256_reader);
                download_configuration.AddReader(md5_reader);

                mf::downloader::Download(
                        url_str, download_configuration, status_callback);
            }
        }
        catch (const std::exception & ex)
        {
            std::cerr << "Error: " << ex.what() << std::endl;
            return 1;
        }
        catch (boost::exception & ex)
        {
            std::cerr << "Error: " << boost::diagnostic_information(ex);
            return 1;
        }

        io_service.run();

        if (success)
        {
            std::cerr << "MD5: " << md5_reader->GetHash() << std::endl;
            std::cerr << "SHA256: " << sha256_reader->GetHash() << std::endl;
        }
    }
    catch (std::exception & e)
    {
        std::cerr << "Uncaught exception: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }

    return 0;
}
