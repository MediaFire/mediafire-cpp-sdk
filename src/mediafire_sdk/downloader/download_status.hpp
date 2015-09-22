/**
 * @file mediafire_sdk/downloader/download_status.hpp
 * @author Herbert Jones
 * @brief Status of a download.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <deque>
#include <string>
#include <system_error>

#include "boost/variant/variant.hpp"
#include "boost/filesystem/path.hpp"

namespace mf
{
namespace downloader
{
namespace status
{
namespace success
{

/** Result of a successful download to the local filesystem. */
struct OnDisk
{
    boost::filesystem::path filepath;
};

/** Result of a successful download to memory. */
struct InMemory
{
    std::shared_ptr<std::deque<uint8_t>> buffer;
};

/** Result of a successful download with no storage location if not provided via
    download readers. */
struct NoTarget
{
};
}  // namespace success
}  // namespace status

using DownloadSuccessType = boost::variant<status::success::OnDisk,
                                           status::success::InMemory,
                                           status::success::NoTarget>;

namespace status
{

/** Sent when download progress is made. */
struct Progress
{
    uint64_t bytes_read;
};

/** Sent on download failure. */
struct Failure
{
    std::error_code error_code;
    std::string description;
};

/** Sent on download success. */
struct Success
{
    DownloadSuccessType success_type;
};

}  // namespace status

/** Variant for download statuses. */
using DownloadStatus
        = boost::variant<status::Progress, status::Failure, status::Success>;

}  // namespace downloader
}  // namespace mf
