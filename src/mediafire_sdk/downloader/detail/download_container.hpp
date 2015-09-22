/**
 * @file mediafire_sdk/downloader/detail/download_container.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "mediafire_sdk/http/http_request.hpp"

#include "mediafire_sdk/downloader/acceptor_interface.hpp"
#include "mediafire_sdk/downloader/callbacks.hpp"
#include "mediafire_sdk/downloader/download_config.hpp"
#include "mediafire_sdk/downloader/download_interface.hpp"
#include "mediafire_sdk/downloader/detail/request_response_proxy.hpp"

#include "mediafire_sdk/utils/fileio.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

class DownloadContainer : public mf::downloader::DownloadInterface,
                          public mf::http::RequestResponseInterface,
                          public std::enable_shared_from_this<DownloadContainer>
{
public:
    enum class State
    {
        Unstarted,
        ReadingExistingFile,
        GetResumeResponseHeader,

        DownloadingFull,
        DownloadingFromOffset,

        Failure,  // Terminate state
        Success,  // Terminate state
    };

    DownloadContainer(std::string url,
                      DownloadConfig download_config,
                      StatusCallback status_callback);

    virtual ~DownloadContainer();

    // Cancellable
    virtual void Cancel() override;

    void Start();

private:
    virtual void RedirectHeaderReceived(const mf::http::Headers & /* headers */,
                                        const mf::http::Url & /* new_url */
                                        ) override;

    virtual void ResponseHeaderReceived(const mf::http::Headers & /* headers */
                                        ) override;

    virtual void ResponseContentReceived(
            std::size_t /* start_pos */,
            std::shared_ptr<mf::http::BufferInterface> /*buffer*/) override;

    virtual void RequestResponseErrorEvent(std::error_code /*error_code*/,
                                           std::string /*error_text*/) override;

    virtual void RequestResponseCompleteEvent() override;

private:
    bool IsState(const State state) const { return state == state_; }

    void StartContinueWritingToFilesystemPath(
            const boost::filesystem::path & download_path,
            uint64_t remote_filesize);
    void StartWriteToFilesystemPath(
            const config::WriteToFilesystemPath & write_config);
    void StartWriteToFilesystemUsingFilenameFromResponseHeader(
            const config::WriteToFilesystemUsingFilenameFromResponseHeader &
                    write_config);
    void StartWriteToMemory(const config::WriteToMemory & write_config);
    void StartWriteNoTaget();

    void ReadExistingFileBeforeContinuingDownload(boost::filesystem::path,
                                                  uint64_t remote_filesize);
    void ContinueReadingExistingFileBeforeContinuingDownload(
            mf::utils::FileIO::Pointer file_io,
            uint64_t bytes_read,
            uint64_t remote_filesize);
    void GetRangedResponseHeader();
    void OpenFileForAppendAndDownloadRange(
            const boost::filesystem::path & download_path,
            uint64_t remote_filesize);

    void DownloadFullFile(
            std::shared_ptr<AcceptorInterface> acceptor_interface);
    void DownloadFileFromOffset(
            std::shared_ptr<AcceptorInterface> acceptor_interface,
            uint64_t offset,
            uint64_t remote_filesize);

    void DownloadFile(
            std::shared_ptr<AcceptorInterface> acceptor_interface,
            std::vector<std::pair<std::string, std::string>> header_pairs);

    void Cleanup();

    void Fail(std::error_code, std::string description);
    void DoFail(std::error_code, std::string description);
    void Complete();
    void TerminateStateCleanup();

    DownloadPointer keep_alive_pointer_;

    State state_;
    std::string url_;
    DownloadConfig download_config_;
    StatusCallback status_callback_;

    std::shared_ptr<AcceptorInterface> acceptor_interface_;
    mf::http::HttpRequest::Pointer http_request_;
    std::shared_ptr<RequestResponseProxy> http_request_proxy_;

    boost::optional<uint64_t> resume_read_bytes_;
    boost::optional<boost::filesystem::path> discovered_path_;
};

}  // namespace detail
}  // namespace downloader
}  // namespace mf
