/**
 * @file buffer_interface.hpp
 * @author Herbert Jones
 * @brief Interface to be used for asynchronously reading remote HTTP response.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cstdint>

namespace mf
{
namespace http
{

/**
 * Interface for buffers used by HttpRequest.
 */
class BufferInterface
{
public:
    BufferInterface() {}
    virtual ~BufferInterface() {}

    /**
     * Get size of data in buffer.
     *
     * @return Size of data in buffer.
     */
    virtual uint64_t Size() const = 0;

    /**
     * @brief Get buffer
     *
     * @return pointer that can be read with data size contained in Size().
     */
    virtual const uint8_t * Data() const = 0;
};

}  // namespace http
}  // namespace mf
