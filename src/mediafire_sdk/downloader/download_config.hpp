/**
 * @file mediafire_sdk/downloader/download_config.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "boost/filesystem/path.hpp"
#include "boost/variant/variant.hpp"

#include "mediafire_sdk/downloader/acceptor_interface.hpp"
#include "mediafire_sdk/downloader/reader/reader_interface.hpp"
#include "mediafire_sdk/http/http_config.hpp"
#include "mediafire_sdk/utils/error_or.hpp"

#include <memory>

namespace mf
{
namespace downloader
{

/** Wraps result of the "select filename from HTTP header filename"
    operation. */
using ErrorOrSaveLocation = mf::utils::ErrorOr<boost::filesystem::path>;

/**
 * @brief Callback signature for selecting the download path using the filename
 * supplied in the HTTP header.
 *
 * @param[in] maybe_filename The filename of the file if supplied in the HTTP
 *                           result header.
 * @param[in] url The URL string of the download.
 * @param[in] header_container The supplied headers already parsed.
 *
 * @return Error or download path.
 */
using SelectFilePathCallback = std::function<ErrorOrSaveLocation(
        boost::optional<std::string> /* filename in HTTP header*/,
        const std::string & /*url*/,
        const mf::http::Headers &)>;

namespace config
{

// Disk targets
// ------------

/** Use to continue a previously started download.  Parameter for
    DownloadConfig. */
struct ContinueWritingToFilesystemPath
{
    boost::variant<boost::filesystem::path, SelectFilePathCallback>
            path_or_select_path_callback;

    boost::optional<uint64_t> expected_filesize;

    /** @todo hjones: Add validator callback - 2015-06-18 */
};

/** Use to download a file to a local filepath.  Parameter for DownloadConfig.
 */
struct WriteToFilesystemPath
{
    enum class FileAction
    {
        RewriteIfExisting,
        FailIfExisting
    };

    FileAction file_action;
    boost::filesystem::path download_path;
};

/** Use to download a file to a local filepath when the path will be determined
    by the filename in the HTTP response header.  Parameter for
    DownloadConfig. */
struct WriteToFilesystemUsingFilenameFromResponseHeader
{
    SelectFilePathCallback select_file_path_callback;
};

// Non-disk targets
// ----------------

/** Use to download a file to memory.  Parameter for DownloadConfig. */
struct WriteToMemory
{
};

/** Use to download a file but not store it anywhere.  Can be used with a custom
    reader to get the hash of a file, or to stream it to another sources.
    Parameter for DownloadConfig. */
struct NoTarget
{
};

}  // namespace config

/**
 * @class DownloadConfig
 * @brief Configuration object for downloads.
 */
class DownloadConfig
{
public:
    using DownloadType = boost::
            variant<config::ContinueWritingToFilesystemPath,
                    config::WriteToFilesystemPath,
                    config::WriteToFilesystemUsingFilenameFromResponseHeader,
                    config::WriteToMemory,
                    config::NoTarget>;

    /**
     * @brief Construct DownloadConfig
     *
     * @param[in] http_config Config for http.
     * @param[in] download_type Config for type of download.
     */
    DownloadConfig(mf::http::HttpConfig::ConstPointer http_config,
                   DownloadType download_type)
            : http_config_(std::move(http_config)),
              download_type_(std::move(download_type))
    {
    }

    /**
     * @brief Set an optional reader, such as Sha256Reader which calculates the
     * sha256 as the file is downloaded.
     *
     * Not required to be set.
     *
     * @param[in] reader Interface which will read the file as it is downloaded.
     */
    void AddReader(std::shared_ptr<ReaderInterface> reader)
    {
        readers_.push_back(reader);
    }

    /**
     * @brief Get the download readers.
     *
     * @return download readers
     */
    const std::vector<std::shared_ptr<ReaderInterface>> & GetReaders() const
    {
        return readers_;
    }

    /**
     * @brief Get the download type.
     *
     * @return download type
     */
    const DownloadType & GetDownloadType() const { return download_type_; }

    /**
     * @brief Get the HttpConfig object that determines connection settings.
     *
     * @return HttpConfig object
     */
    mf::http::HttpConfig::ConstPointer GetHttpConfig() const
    {
        return http_config_;
    }

private:
    mf::http::HttpConfig::ConstPointer http_config_;
    DownloadType download_type_;

    std::vector<std::shared_ptr<ReaderInterface>> readers_;
};

}  // namespace downloader
}  // namespace mf
