/**
 * @file mediafire_sdk/downloader/error/conditions/generic.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "generic.hpp"

#include <cassert>
#include <sstream>
#include <string>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace
{
/**
 * @class ApiConditionImpl
 * @brief std::error_category implementation for api namespace
 */
class ApiConditionImpl : public std::error_category
{
public:
    /// The name of this error category.
    virtual const char * name() const NOEXCEPT;

    /// The message belonging to the error code.
    virtual std::string message(int ev) const;

    /**
     * @brief Compare other error codes to your error conditions/values, or
     *        pass them to other comparers.
     *
     * Any of your error codes that correspond to std::errc types should be
     * matched down here, and return true if so.
     *
     * See: http://en.cppreference.com/w/cpp/error/errc
     */
    virtual bool equivalent(const std::error_code & code,
                            int condition) const NOEXCEPT;
};

const char * ApiConditionImpl::name() const NOEXCEPT
{
    return "mediafire downloader";
}

std::string ApiConditionImpl::message(int ev) const
{
    using mf::downloader::errc;

    switch (static_cast<errc>(ev))
    {
        case errc::IncompleteWrite:
            return "incomplete write";
        case errc::Cancelled:
            return "cancelled";
        case errc::OverwriteDenied:
            return "overwrite denied";
        case errc::NoFilenameInHeader:
            return "no filename in header";
        case errc::DownloadResumeUnsupported:
            return "download resume unsupported";
        case errc::DownloadResumeHeadersMissing:
            return "download resume headers missing";
        case errc::ResumedDownloadChangedRemotely:
            return "resumed download changed remotely";
        case errc::ResumedDownloadChangedLocally:
            return "resumed download changed locally";
        case errc::BadHttpStatus:
            return "bad http status";
        case errc::ResumedDownloadAlreadyDownloaded:
            return "resumed download already downloaded";
        case errc::ResumedDownloadTooLarge:
            return "resumed download too large";
        default:
        {
            assert(!"Unimplemented message");
            std::stringstream ss;
            ss << "Unknown downloader category: " << ev;
            return ss.str();
        }
    }
}

bool ApiConditionImpl::equivalent(const std::error_code & /*ec*/,
                                  int condition_code) const NOEXCEPT
{
    using mf::downloader::errc;

    switch (static_cast<errc>(condition_code))
    {
        default:
            return false;
    }
}
}  // namespace

namespace mf
{
namespace downloader
{

std::error_condition make_error_condition(errc e)
{
    return std::error_condition(static_cast<int>(e),
                                generic_download_category());
}

std::error_code make_error_code(errc e)
{
    return std::error_code(static_cast<int>(e), generic_download_category());
}

const std::error_category & generic_download_category()
{
    static ApiConditionImpl instance;
    return instance;
}

}  // namespace downloader
}  // namespace mf
