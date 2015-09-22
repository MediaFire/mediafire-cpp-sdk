/**
 * @file mediafire_sdk/downloader/error/conditions/generic.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

namespace mf
{
namespace downloader
{

/**
 * Downloader error conditions
 *
 * You may compare these to API errors and they may be used as generic errors.
 */
enum class errc
{
    IncompleteWrite = 1,  // Can't use 0
    Cancelled,
    OverwriteDenied,
    NoFilenameInHeader,
    DownloadResumeUnsupported,
    DownloadResumeHeadersMissing,
    ResumedDownloadChangedRemotely,
    ResumedDownloadChangedLocally,
    BadHttpStatus,
    ResumedDownloadAlreadyDownloaded,
    ResumedDownloadTooLarge,
};

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(errc e);

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(errc e);

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category & generic_download_category();

}  // End namespace downloader
}  // namespace mf

namespace std
{
template <>
struct is_error_condition_enum<mf::downloader::errc> : public true_type
{
};
}  // End namespace std
