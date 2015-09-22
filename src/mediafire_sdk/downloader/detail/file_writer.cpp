/**
 * @file mediafire_sdk/downloader/file_writer.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "file_writer.hpp"

#include "mediafire_sdk/downloader/error.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

FileWriter::FileWriter(mf::utils::FileIO::Pointer file) : file_(file) {}

void FileWriter::AcceptRejectionCallback(RejectionCallback callback)
{
    fail_download_ = callback;
}

void FileWriter::ClearRejectionCallback() { fail_download_.reset(); }

void FileWriter::RedirectHeaderReceived(const mf::http::Headers & /* headers */,
                                        const mf::http::Url & /* new_url */)
{
}

void FileWriter::ResponseHeaderReceived(const mf::http::Headers & /* headers */)
{
}

void FileWriter::ResponseContentReceived(
        std::size_t /*start_pos*/,
        std::shared_ptr<mf::http::BufferInterface> buffer)
{
    if (file_)
    {
        auto ec = std::error_code();
        auto size_written = file_->Write(buffer->Data(), buffer->Size(), &ec);

        if (ec)
        {
            if (fail_download_)
                (*fail_download_)(ec, "Failed writing to file.");
        }
        else if (size_written != buffer->Size())
        {
            if (fail_download_)
                (*fail_download_)(
                        make_error_code(mf::downloader::errc::IncompleteWrite),
                        "Failed writing complete data to file.");
        }
    }
}

void FileWriter::RequestResponseErrorEvent(std::error_code /*error_code*/,
                                           std::string /*error_text*/)
{
}

void FileWriter::RequestResponseCompleteEvent()
{
    if (file_)
    {
        // This will close the file.
        file_.reset();
    }
}

}  // namespace detail
}  // namespace downloader
}  // namespace mf
