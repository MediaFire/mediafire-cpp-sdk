/**
 * @file mediafire_sdk/downloader/detail/download_container.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "download_container.hpp"

#include <functional>

#include "boost/filesystem/operations.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/downloader/detail/file_writer.hpp"
#include "mediafire_sdk/downloader/detail/header_utils.hpp"
#include "mediafire_sdk/downloader/detail/memory_writer.hpp"
#include "mediafire_sdk/downloader/detail/no_target_writer.hpp"
#include "mediafire_sdk/downloader/detail/unknown_name_file_writer.hpp"
#include "mediafire_sdk/downloader/error.hpp"
#include "mediafire_sdk/utils/variant.hpp"
#include "mediafire_sdk/utils/error.hpp"

using mf::utils::Match;
using mf::utils::MatchPartial;
using State = mf::downloader::detail::DownloadContainer::State;
using mf::downloader::errc;

namespace
{
bool IsTerminateState(State state)
{
    switch (state)
    {
        case State::Failure:
        case State::Success:
            return true;
        default:
            return false;
    }
}
bool IsDownloadingState(State state)
{
    switch (state)
    {
        case State::DownloadingFull:
        case State::DownloadingFromOffset:
            return true;
        default:
            return false;
    }
}
std::shared_ptr<mf::downloader::detail::RequestResponseProxy> MakeProxy(
        std::shared_ptr<mf::downloader::detail::DownloadContainer>
                download_container)
{
    return std::make_shared<mf::downloader::detail::RequestResponseProxy>(
            std::static_pointer_cast<mf::http::RequestResponseInterface>(
                    download_container));
}
}  // namespace

namespace mf
{
namespace downloader
{
namespace detail
{

using namespace mf::downloader::config;

DownloadContainer::DownloadContainer(std::string url,
                                     DownloadConfig download_config,
                                     StatusCallback status_callback)
        : state_(State::Unstarted),
          url_(url),
          download_config_(download_config),
          status_callback_(status_callback)
{
}

DownloadContainer::~DownloadContainer() {}

void DownloadContainer::Cancel()
{
    if (http_request_)
        http_request_->Fail(make_error_code(mf::downloader::errc::Cancelled),
                            "Download cancelled");

    Cleanup();
}

void DownloadContainer::Cleanup()
{
    keep_alive_pointer_.reset();  // Leave last.
}

void DownloadContainer::Start()
{
    keep_alive_pointer_ = shared_from_this();

    Match(download_config_.GetDownloadType(),
          [this](const ContinueWritingToFilesystemPath &) -> void
          {
              GetRangedResponseHeader();
          },
          [this](const WriteToFilesystemPath & write_config) -> void
          {
              StartWriteToFilesystemPath(write_config);
          },
          [this](const WriteToFilesystemUsingFilenameFromResponseHeader &
                         write_config) -> void
          {
              StartWriteToFilesystemUsingFilenameFromResponseHeader(
                      write_config);
          },
          [this](const WriteToMemory & write_config) -> void
          {
              StartWriteToMemory(write_config);
          },
          [this](const NoTarget &) -> void
          {
              DownloadFullFile(std::make_shared<detail::NoTargetWriter>());
          });
}

void DownloadContainer::StartContinueWritingToFilesystemPath(
        const boost::filesystem::path & download_path,
        uint64_t remote_filesize)
{
    discovered_path_ = download_path;
    const auto exists = boost::filesystem::exists(download_path);
    if (exists && !download_config_.GetReaders().empty())
    {
        ReadExistingFileBeforeContinuingDownload(download_path,
                                                 remote_filesize);
    }
    else if (exists)
    {
        OpenFileForAppendAndDownloadRange(*discovered_path_, remote_filesize);
    }
    else
    {
        auto ec = std::error_code();
        auto file_io = mf::utils::FileIO::Open(download_path, "wb", &ec);
        if (ec)
        {
            Fail(ec, "Failed to open file.");
        }
        else
        {
            DownloadFullFile(std::make_shared<detail::FileWriter>(file_io));
        }
    }
}

void DownloadContainer::OpenFileForAppendAndDownloadRange(
        const boost::filesystem::path & download_path,
        const uint64_t remote_filesize)
{
    auto ec = std::error_code();
    auto file_io = mf::utils::FileIO::Open(download_path, "ab", &ec);
    if (ec)
    {
        Fail(ec, "Failed to open file.");
    }
    else
    {
        const auto local_filesize = file_io->Tell();
        if (resume_read_bytes_ && *resume_read_bytes_ != local_filesize)
        {
            Fail(make_error_code(errc::ResumedDownloadChangedLocally),
                 "Unexpected filesize in resumed download.");
        }
        else if (local_filesize == remote_filesize)
        {
            Fail(make_error_code(errc::ResumedDownloadAlreadyDownloaded),
                 "Local filesize matches remote filesize.");
        }
        else if (local_filesize > remote_filesize)
        {
            Fail(make_error_code(errc::ResumedDownloadTooLarge),
                 "Local filesize larger than remote filesize.");
        }
        else
        {
            DownloadFileFromOffset(
                    std::make_shared<detail::FileWriter>(file_io),
                    local_filesize,
                    remote_filesize);
        }
    }
}

void DownloadContainer::StartWriteToFilesystemPath(
        const config::WriteToFilesystemPath & write_config)
{
    using FileAction = WriteToFilesystemPath::FileAction;

    if (write_config.file_action == FileAction::FailIfExisting)
    {
        if (boost::filesystem::exists(write_config.download_path))
        {
            return Fail(make_error_code(errc::OverwriteDenied),
                        "Download to existing file denied.");
        }
    }

    auto ec = std::error_code();
    auto file_io
            = mf::utils::FileIO::Open(write_config.download_path, "wb", &ec);
    if (ec)
    {
        Fail(ec, "Failed to open file.");
    }
    else
    {
        DownloadFullFile(std::make_shared<detail::FileWriter>(file_io));
    }
}

void DownloadContainer::StartWriteToFilesystemUsingFilenameFromResponseHeader(
        const config::WriteToFilesystemUsingFilenameFromResponseHeader & writer)
{
    DownloadFullFile(std::make_shared<detail::UnknownNameFileWriter>(
            url_, writer.select_file_path_callback));
}

void DownloadContainer::StartWriteToMemory(const config::WriteToMemory &)
{
    DownloadFullFile(std::make_shared<detail::MemoryWriter>());
}

void DownloadContainer::StartWriteNoTaget()
{
    DownloadFullFile(std::make_shared<detail::NoTargetWriter>());
}

void DownloadContainer::ReadExistingFileBeforeContinuingDownload(
        boost::filesystem::path filepath,
        uint64_t remote_filesize)
{
    auto ec = std::error_code();
    auto file_io = mf::utils::FileIO::Open(filepath, "rb", &ec);
    if (ec)
    {
        Fail(ec, "Failed to open file.");
    }
    else
    {
        state_ = State::ReadingExistingFile;
        resume_read_bytes_ = 0;
        ContinueReadingExistingFileBeforeContinuingDownload(
                file_io, 0, remote_filesize);
    }
}

void DownloadContainer::ContinueReadingExistingFileBeforeContinuingDownload(
        mf::utils::FileIO::Pointer file_io,
        uint64_t total_bytes_read,
        uint64_t remote_filesize)
{
    if (IsState(State::ReadingExistingFile))
    {
        assert(resume_read_bytes_);
        assert(discovered_path_);

        const auto buffer_size = uint64_t{1024 * 8};
        uint8_t buffer[buffer_size];
        auto error_code = std::error_code();
        const auto bytes_read = file_io->Read(buffer, buffer_size, &error_code);

        if (bytes_read > 0)
        {
            *resume_read_bytes_ += bytes_read;
            for (auto & reader : download_config_.GetReaders())
                reader->HandleData(bytes_read, buffer);
        }

        if (error_code)
        {
            if (error_code == mf::utils::file_io_error::EndOfFile)
            {
                OpenFileForAppendAndDownloadRange(*discovered_path_,
                                                  remote_filesize);
            }
            else
            {
                Fail(error_code, "Error while reading existing file.");
            }
        }
        else
        {
            // Continue reading without hogging resources
            download_config_.GetHttpConfig()
                    ->GetDefaultCallbackIoService()
                    ->post(std::bind(
                            &DownloadContainer::
                                    ContinueReadingExistingFileBeforeContinuingDownload,
                            shared_from_this(),
                            file_io,
                            total_bytes_read + bytes_read,
                            remote_filesize));
        }
    }
}

void DownloadContainer::GetRangedResponseHeader()
{
    if (!IsTerminateState(state_))
    {
        state_ = State::GetResumeResponseHeader;

        http_request_proxy_ = MakeProxy(shared_from_this());

        http_request_ = mf::http::HttpRequest::Create(
                download_config_.GetHttpConfig(), http_request_proxy_, url_);

        http_request_->SetHeader("Range", "bytes=0-");
        http_request_->SetRequestHeadersOnly();

        http_request_->Start();
    }
}

void DownloadContainer::DownloadFullFile(
        std::shared_ptr<AcceptorInterface> acceptor_interface)
{
    if (!IsTerminateState(state_))
    {
        state_ = State::DownloadingFull;

        DownloadFile(std::move(acceptor_interface),
                     std::vector<std::pair<std::string, std::string>>());
    }
}

void DownloadContainer::DownloadFileFromOffset(
        std::shared_ptr<AcceptorInterface> acceptor_interface,
        uint64_t offset,
        uint64_t remote_filesize)
{
    if (!IsTerminateState(state_))
    {
        auto header_pairs = std::vector<std::pair<std::string, std::string>>();
        auto header_name = std::string("Range");
        auto value = std::string("bytes=")
                     + boost::lexical_cast<std::string>(offset) + "-"
                     + boost::lexical_cast<std::string>(remote_filesize);

        header_pairs.emplace_back(std::move(header_name), std::move(value));

        state_ = State::DownloadingFromOffset;

        DownloadFile(std::move(acceptor_interface), header_pairs);
    }
}

void DownloadContainer::DownloadFile(
        std::shared_ptr<AcceptorInterface> acceptor_interface,
        std::vector<std::pair<std::string, std::string>> header_pairs)
{
    assert(IsDownloadingState(state_));
    if (IsDownloadingState(state_))
    {
        using namespace std::placeholders;  // for _1, _2, _3...

        acceptor_interface_ = std::move(acceptor_interface);
        assert(acceptor_interface_);

        acceptor_interface_->AcceptRejectionCallback(std::bind(
                &DownloadContainer::Fail, shared_from_this(), _1, _2));

        http_request_proxy_ = MakeProxy(shared_from_this());

        http_request_ = mf::http::HttpRequest::Create(
                download_config_.GetHttpConfig(), http_request_proxy_, url_);

        for (const auto & pair : header_pairs)
            http_request_->SetHeader(pair.first, pair.second);

        http_request_->Start();
    }
}

void DownloadContainer::RedirectHeaderReceived(
        const mf::http::Headers & headers_container,
        const mf::http::Url & new_url)
{
    if (IsDownloadingState(state_))
    {
        assert(acceptor_interface_);

        if (acceptor_interface_)
            acceptor_interface_->RedirectHeaderReceived(headers_container,
                                                        new_url);
    }
}

void DownloadContainer::ResponseHeaderReceived(
        const mf::http::Headers & headers_container)
{
    if (IsState(State::GetResumeResponseHeader))
    {
        // Stop reading the response.
        http_request_proxy_->Reset();  // Ignore errors from http_request_
        http_request_->Cancel();
        http_request_.reset();

        if (headers_container.status_code != 200
            && headers_container.status_code != 206)
        {
            Fail(make_error_code(errc::DownloadResumeUnsupported),
                 "Improper status code from resume headers.");
        }
        else
        {
            const auto & write_config
                    = boost::get<ContinueWritingToFilesystemPath>(
                            download_config_.GetDownloadType());
            const auto & header_map = headers_container.headers;

            auto content_length_it = header_map.find("content-length");
            auto accept_ranges_it = header_map.find("accept-ranges");
            if (content_length_it == header_map.end())
            {
                Fail(make_error_code(errc::DownloadResumeUnsupported),
                     "No content length in response.");
            }
            else if (accept_ranges_it != header_map.end()
                     && accept_ranges_it->second == "none")
            {
                // Servers are free to not have this header.  However if it is
                // set and is set to "none", then ranges are rejected.
                Fail(make_error_code(errc::DownloadResumeUnsupported),
                     "Ranges denied by header.");
            }
            else
            {
                try
                {
                    const auto content_length = boost::lexical_cast<uint64_t>(
                            content_length_it->second);

                    // validate filesize if possible
                    if (write_config.expected_filesize
                        && *write_config.expected_filesize != content_length)
                    {
                        Fail(make_error_code(
                                     errc::ResumedDownloadChangedRemotely),
                             "Unexpected filesize in resumed download.");
                    }
                    else
                    {
                        Match(write_config.path_or_select_path_callback,
                              [&, this](const boost::filesystem::path & path)
                              {
                                  StartContinueWritingToFilesystemPath(
                                          path, content_length);
                              },
                              [&, this](const SelectFilePathCallback & callback)
                              {
                                  auto filename = detail::FilenameFromHeaders(
                                          headers_container);
                                  const auto error_or_path = callback(
                                          filename, url_, headers_container);

                                  if (error_or_path.IsError())
                                  {
                                      Fail(error_or_path.GetErrorCode(),
                                           error_or_path.GetDescription());
                                  }
                                  else
                                  {
                                      StartContinueWritingToFilesystemPath(
                                              error_or_path.GetValue(),
                                              content_length);
                                  }
                              });
                    }
                }
                catch (boost::bad_lexical_cast & ex)
                {
                    std::string message(
                            "Improper filesize in resume response headers: ");
                    message += content_length_it->second;
                    Fail(make_error_code(errc::DownloadResumeUnsupported),
                         message);
                }
            }
        }
    }
    else if (IsState(State::DownloadingFromOffset)
             && headers_container.status_code != 206)
    {
        auto message = std::string("Unexpected HTTP status: ");
        message += boost::lexical_cast<std::string>(
                headers_container.status_code);
        message += " (expected 206)";
        Fail(make_error_code(errc::BadHttpStatus), message);
    }
    else if (IsState(State::DownloadingFull)
             && headers_container.status_code != 200)
    {
        auto message = std::string("Unexpected HTTP status: ");
        message += boost::lexical_cast<std::string>(
                headers_container.status_code);
        message += " (expected 200)";
        Fail(make_error_code(errc::BadHttpStatus), message);
    }
    else if (IsDownloadingState(state_))
    {
        if (acceptor_interface_)
            acceptor_interface_->ResponseHeaderReceived(headers_container);
    }
}

void DownloadContainer::ResponseContentReceived(
        std::size_t start_pos,
        std::shared_ptr<mf::http::BufferInterface> buffer)
{
    if (IsDownloadingState(state_))
    {
        assert(acceptor_interface_);

        for (auto & reader : download_config_.GetReaders())
        {
            reader->HandleData(buffer->Size(), buffer->Data());
        }

        if (acceptor_interface_)
            acceptor_interface_->ResponseContentReceived(start_pos, buffer);

        auto end_byte_pos = start_pos + buffer->Size();

        status_callback_(status::Progress{end_byte_pos});
    }
}

void DownloadContainer::RequestResponseErrorEvent(std::error_code error_code,
                                                  std::string error_text)
{
    if (!IsTerminateState(state_))
    {
        if (acceptor_interface_)
        {
            // Clear the rejection callback so that the interface can't call
            // Fail, as we will call it afterwards anyway.
            acceptor_interface_->ClearRejectionCallback();

            acceptor_interface_->RequestResponseErrorEvent(error_code,
                                                           error_text);
        }

        Fail(error_code, error_text);
    }
}

void DownloadContainer::RequestResponseCompleteEvent()
{
    if (IsDownloadingState(state_))
    {
        assert(acceptor_interface_);

        if (acceptor_interface_)
            acceptor_interface_->RequestResponseCompleteEvent();

        Complete();

        http_request_.reset();
    }
}

void DownloadContainer::Fail(std::error_code error_code,
                             std::string description)
{
    // Prevent potential unwanted recursion.
    download_config_.GetHttpConfig()->GetDefaultCallbackIoService()->post(
            std::bind(&DownloadContainer::DoFail,
                      shared_from_this(),
                      error_code,
                      description));
}

void DownloadContainer::DoFail(std::error_code error_code,
                               std::string description)
{
    if (!IsTerminateState(state_))
    {
        state_ = State::Failure;

        status_callback_(status::Failure{error_code, description});

        TerminateStateCleanup();
    }
}

void DownloadContainer::Complete()
{
    if (!IsTerminateState(state_))
    {
        state_ = State::Success;

        Match(download_config_.GetDownloadType(),
              [this](const ContinueWritingToFilesystemPath & config) -> void
              {
                  Match(config.path_or_select_path_callback,
                        [&, this](const boost::filesystem::path & path)
                        {
                            status_callback_(status::Success{
                                    status::success::OnDisk{path}});
                        },
                        [&, this](const SelectFilePathCallback &)
                        {
                            status_callback_(
                                    status::Success{status::success::OnDisk{
                                            *discovered_path_}});

                        });
              },
              [this](const WriteToFilesystemPath & config) -> void
              {
                  status_callback_(status::Success{
                          status::success::OnDisk{config.download_path}});
              },
              [this](const WriteToFilesystemUsingFilenameFromResponseHeader &)
                      -> void
              {
                  auto & path
                          = std::static_pointer_cast<detail::UnknownNameFileWriter>(
                                    acceptor_interface_)->GetDownloadPath();

                  status_callback_(
                          status::Success{status::success::OnDisk{path}});
              },
              [this](const WriteToMemory &) -> void
              {
                  assert(acceptor_interface_);
                  auto memory_writer
                          = std::static_pointer_cast<detail::MemoryWriter>(
                                  acceptor_interface_);

                  status_callback_(status::Success{status::success::InMemory{
                          memory_writer->GetBuffer()}});
              },
              [this](const NoTarget &) -> void
              {
                  status_callback_(
                          status::Success{status::success::NoTarget{}});
              });

        TerminateStateCleanup();
    }
}

void DownloadContainer::TerminateStateCleanup()
{
    if (http_request_proxy_)
    {
        http_request_proxy_->Reset();
        http_request_proxy_.reset();
    }
    if (http_request_)
    {
        http_request_->Cancel();
        http_request_.reset();
    }

    if (acceptor_interface_)
    {
        acceptor_interface_->ClearRejectionCallback();
        acceptor_interface_.reset();
    }

    status_callback_ = [](DownloadStatus)
    {
    };
}

}  // namespace detail
}  // namespace downloader
}  // namespace mf
