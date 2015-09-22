/**
 * @file mediafire_sdk/downloader/file_writer.hpp
 * @author Herbert Jones
 * @brief Write http request to file.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

#include "mediafire_sdk/http/request_response_interface.hpp"
#include "mediafire_sdk/http/http_config.hpp"

#include "mediafire_sdk/downloader/acceptor_interface.hpp"
#include "mediafire_sdk/downloader/callbacks.hpp"
#include "mediafire_sdk/downloader/detail/file_writer.hpp"
#include "mediafire_sdk/downloader/download_config.hpp"

#include "mediafire_sdk/utils/error_or.hpp"
#include "mediafire_sdk/utils/fileio.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

class UnknownNameFileWriter : public AcceptorInterface
{
public:
    UnknownNameFileWriter(std::string url,
                          SelectFilePathCallback select_file_path_callback);

    virtual void AcceptRejectionCallback(RejectionCallback) override;

    virtual void ClearRejectionCallback() override;

    virtual void RedirectHeaderReceived(
            const mf::http::Headers & /* headers */,
            const mf::http::Url & /* new_url */) override;

    virtual void ResponseHeaderReceived(
            const mf::http::Headers & /* headers */) override;

    virtual void ResponseContentReceived(
            std::size_t /* start_pos */,
            std::shared_ptr<mf::http::BufferInterface> /*buffer*/) override;

    virtual void RequestResponseErrorEvent(std::error_code /*error_code*/,
                                           std::string /*error_text*/) override;

    virtual void RequestResponseCompleteEvent() override;

    const boost::filesystem::path GetDownloadPath() const
    {
        return download_path_;
    }

private:
    std::string url_;
    SelectFilePathCallback select_file_path_callback_;

    boost::optional<RejectionCallback> fail_download_;
    std::shared_ptr<FileWriter> file_writer_;

    boost::filesystem::path download_path_;
};

}  // namespace detail
}  // namespace downloader
}  // namespace mf
