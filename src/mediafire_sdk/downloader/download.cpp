/**
 * @file mediafire_sdk/downloader/download.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "download.hpp"

#include "mediafire_sdk/downloader/detail/file_writer.hpp"
#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/downloader/detail/download_container.hpp"

namespace mf
{
namespace downloader
{

DownloadPointer Download(std::string url,
                         DownloadConfig download_config,
                         StatusCallback status_callback)
{
    auto dld = std::make_shared<detail::DownloadContainer>(
            std::move(url),
            std::move(download_config),
            std::move(status_callback));

    dld->Start();
    return dld;
}

}  // namespace downloader
}  // namespace mf
