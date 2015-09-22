/**
 * @file mediafire_sdk/downloader/download_interface.hpp
 * @author Herbert Jones
 * @brief Interface for downloaders.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "mediafire_sdk/utils/cancellable_interface.hpp"

#include <memory>

namespace mf
{
namespace downloader
{

/**
 * @class DownloadInterface
 * @brief Interface for downloaders which provides the ability to cancel
 *        downloads.
 */
class DownloadInterface : public mf::utils::CancellableInterface
{
public:
    virtual ~DownloadInterface(){};

    /**
     * @brief Cancel the download.
     */
    virtual void Cancel() override = 0;
};

using DownloadPointer = std::shared_ptr<DownloadInterface>;

}  // namespace downloader
}  // namespace mf
