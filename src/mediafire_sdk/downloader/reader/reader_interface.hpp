/**
 * @file mediafire_sdk/downloader/reader/reader_interface.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <cstdint>

namespace mf
{
namespace downloader
{

/**
 * @class ReaderInterface
 * @brief Interface for reading the contents of a download while it is still
 *        in progress.
 */
class ReaderInterface
{
public:
    virtual ~ReaderInterface(){};

    /**
     * @brief Called when download data is read from an existing file that is
     *        being continued and from downloaded chunks of data.
     *
     * @param[in] size The size of the buffer pointed to by data.
     * @param[in] data Pointer to buffer with downloaded bytes..
     */
    virtual void HandleData(uint64_t size, const uint8_t * data) = 0;
};

}  // namespace downloader
}  // namespace mf
