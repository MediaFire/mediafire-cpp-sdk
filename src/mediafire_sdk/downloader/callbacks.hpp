/**
 * @file mediafire_sdk/downloader/callbacks.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <functional>

#include "mediafire_sdk/downloader/download_status.hpp"

namespace mf
{
namespace downloader
{

/**
 * Signature of the function that will receive the download status as it
 * changes.
 */
using StatusCallback = std::function<void(DownloadStatus)>;

}  // namespace downloader
}  // namespace mf
