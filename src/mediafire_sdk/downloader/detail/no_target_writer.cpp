/**
 * @file mediafire_sdk/downloader/file_writer.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "no_target_writer.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

NoTargetWriter::NoTargetWriter() {}

void NoTargetWriter::AcceptRejectionCallback(RejectionCallback callback)
{
    fail_download_ = callback;
}

void NoTargetWriter::ClearRejectionCallback() { fail_download_.reset(); }

void NoTargetWriter::RedirectHeaderReceived(
        const mf::http::Headers & /* headers */,
        const mf::http::Url & /* new_url */)
{
}

void NoTargetWriter::ResponseHeaderReceived(
        const mf::http::Headers & /* headers */)
{
}

void NoTargetWriter::ResponseContentReceived(
        std::size_t /*start_pos*/,
        std::shared_ptr<mf::http::BufferInterface> /*buffer*/)
{
}

void NoTargetWriter::RequestResponseErrorEvent(std::error_code /*error_code*/,
                                               std::string /*error_text*/)
{
}

void NoTargetWriter::RequestResponseCompleteEvent() {}

}  // namespace detail
}  // namespace downloader
}  // namespace mf
