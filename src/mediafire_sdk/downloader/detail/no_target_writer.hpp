/**
 * @file mediafire_sdk/downloader/detail/memory_writer.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <deque>
#include <string>

#include "boost/filesystem/path.hpp"

#include "mediafire_sdk/http/request_response_interface.hpp"
#include "mediafire_sdk/http/http_config.hpp"

#include "mediafire_sdk/downloader/acceptor_interface.hpp"
#include "mediafire_sdk/downloader/callbacks.hpp"

#include "mediafire_sdk/utils/error_or.hpp"
#include "mediafire_sdk/utils/fileio.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

class NoTargetWriter : public AcceptorInterface
{
public:
    NoTargetWriter();

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

private:
    boost::optional<RejectionCallback> fail_download_;
};

}  // namespace detail
}  // namespace downloader
}  // namespace mf
