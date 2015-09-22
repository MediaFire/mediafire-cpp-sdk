/**
 * @file mediafire_sdk/downloader/file_writer.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "unknown_name_file_writer.hpp"

#include "boost/filesystem/operations.hpp"

#include "mediafire_sdk/downloader/error.hpp"
#include "mediafire_sdk/downloader/detail/header_utils.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

UnknownNameFileWriter::UnknownNameFileWriter(
        std::string url,
        SelectFilePathCallback select_file_path_callback)
        : url_(std::move(url)),
          select_file_path_callback_(std::move(select_file_path_callback))
{
}

void UnknownNameFileWriter::AcceptRejectionCallback(RejectionCallback callback)
{
    fail_download_ = callback;
    if (file_writer_)
        file_writer_->AcceptRejectionCallback(callback);
}

void UnknownNameFileWriter::ClearRejectionCallback()
{
    fail_download_.reset();

    if (file_writer_)
        file_writer_->ClearRejectionCallback();
}

void UnknownNameFileWriter::RedirectHeaderReceived(
        const mf::http::Headers & /*headers*/,
        const mf::http::Url & /* new_url */)
{
    // const auto & header_map = headers.headers;

    // for (const auto & pair : header_map)
    // {
    //     std::cout << pair.first << ": " << pair.second << std::endl;
    // }
}

void UnknownNameFileWriter::ResponseHeaderReceived(
        const mf::http::Headers & headers)
{

    auto filename = FilenameFromHeaders(headers);

    const auto error_or_path
            = select_file_path_callback_(filename, url_, headers);

    if (error_or_path.IsError())
    {
        if (fail_download_)
        {
            (*fail_download_)(error_or_path.GetErrorCode(),
                              error_or_path.GetDescription());
        }
    }
    else
    {
        const auto & download_path = error_or_path.GetValue();

        auto ec = std::error_code();
        auto file_io = mf::utils::FileIO::Open(download_path, "wb", &ec);

        if (ec)
        {
            if (fail_download_)
            {
                std::string description = "Failed to open path: ";
                description += download_path.string();

                (*fail_download_)(ec, description);
            }
        }
        else
        {
            download_path_ = download_path;
            file_writer_ = std::make_shared<detail::FileWriter>(file_io);
        }
    }
}

void UnknownNameFileWriter::ResponseContentReceived(
        std::size_t start_pos,
        std::shared_ptr<mf::http::BufferInterface> buffer)
{
    if (file_writer_)
        file_writer_->ResponseContentReceived(start_pos, buffer);
}

void UnknownNameFileWriter::RequestResponseErrorEvent(
        std::error_code error_code,
        std::string error_text)
{
    if (file_writer_)
        file_writer_->RequestResponseErrorEvent(error_code, error_text);
}

void UnknownNameFileWriter::RequestResponseCompleteEvent()
{
    if (file_writer_)
        file_writer_->RequestResponseCompleteEvent();
}

}  // namespace detail
}  // namespace downloader
}  // namespace mf
