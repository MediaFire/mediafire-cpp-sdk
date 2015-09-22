/**
 * @file mediafire_sdk/downloader/file_writer.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "memory_writer.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

MemoryWriter::MemoryWriter() : buffer_(std::make_shared<std::deque<uint8_t>>())
{
}

void MemoryWriter::AcceptRejectionCallback(RejectionCallback callback)
{
    fail_download_ = callback;
}

void MemoryWriter::ClearRejectionCallback() { fail_download_.reset(); }

void MemoryWriter::RedirectHeaderReceived(
        const mf::http::Headers & /* headers */,
        const mf::http::Url & /* new_url */)
{
}

void MemoryWriter::ResponseHeaderReceived(
        const mf::http::Headers & /* headers */)
{
}

void MemoryWriter::ResponseContentReceived(
        std::size_t /*start_pos*/,
        std::shared_ptr<mf::http::BufferInterface> buffer)
{
    std::copy(buffer->Data(),
              buffer->Data() + buffer->Size(),
              std::back_inserter(*buffer_));
}

void MemoryWriter::RequestResponseErrorEvent(std::error_code /*error_code*/,
                                             std::string /*error_text*/)
{
}

void MemoryWriter::RequestResponseCompleteEvent() {}

}  // namespace detail
}  // namespace downloader
}  // namespace mf
