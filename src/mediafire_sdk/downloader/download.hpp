/**
 * @file mediafire_sdk/downloader/download.hpp
 * @author Herbert Jones
 * @brief Download starters
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "boost/filesystem/path.hpp"

#include "mediafire_sdk/downloader/callbacks.hpp"
#include "mediafire_sdk/downloader/download_config.hpp"
#include "mediafire_sdk/downloader/download_interface.hpp"
#include "mediafire_sdk/downloader/acceptor_interface.hpp"

namespace mf
{
namespace downloader
{

/**
 * @brief Create a download request.
 *
 * @param[in] url The URL to download.
 * @param[in] download_config Download settings.
 * @param[in] callback Function called with status updates.
 *
 * @return New download request.  Can be used to cancel the download.
 */
DownloadPointer Download(std::string url,
                         DownloadConfig download_config,
                         StatusCallback callback);

}  // namespace downloader
}  // namespace mf
