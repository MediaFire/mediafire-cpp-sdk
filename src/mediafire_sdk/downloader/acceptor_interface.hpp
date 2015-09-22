/**
 * @file mediafire_sdk/downloader/acceptor_interface.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <memory>
#include <string>
#include <system_error>

#include "boost/optional/optional.hpp"

#include "mediafire_sdk/http/request_response_interface.hpp"

namespace mf
{
namespace downloader
{

/**
 * @class AcceptorInterface
 * @brief Interface that accepts the content of the download.
 *
 * This can be used to download to a file, or save to memory.
 */
class AcceptorInterface : public mf::http::RequestResponseInterface
{
public:
    virtual ~AcceptorInterface(){};

    using RejectionCallback
            = std::function<void(std::error_code, std::string /*description*/)>;

    /**
     * @brief Set the rejection callback.
     *
     * The acceptor may need to reject accepting the download for any reason.
     * Before the download begins this function will be given a callback it can
     * use to cancel the download.
     */
    virtual void AcceptRejectionCallback(RejectionCallback) = 0;

    /**
     * @brief Erase the rejection callback.
     *
     * This is necessary in case of circular shared pointers in the callbacks.
     */
    virtual void ClearRejectionCallback() = 0;

    /**
     * @brief Called after response header is parsed with redirect directions
     * which are being followed.
     *
     * @param[in] raw_header The headers in plain text.
     * @param[in] headers Headers parsed into parts.
     * @param[in] new_url New request target.
     */
    virtual void RedirectHeaderReceived(const mf::http::Headers & /* headers */,
                                        const mf::http::Url & /* new_url */
                                        ) override = 0;

    /**
     * @brief Called after response header is parsed.
     *
     * This is called once for the response.  If there are any redirects they
     * are received through RedirectHeaderReceived.
     *
     * @param[in] headers Headers parsed into parts.
     */
    virtual void ResponseHeaderReceived(const mf::http::Headers & /* headers */
                                        ) override = 0;

    /**
     * @brief Called when content received.
     *
     * As content is streamed from the remote server, this is called with the
     * streamed content.
     *
     * @param[in] start_pos Where in the response content the buffer starts.
     * @param[in] buffer The streamed data from the remote server.
     */
    virtual void ResponseContentReceived(
            std::size_t /* start_pos */,
            std::shared_ptr<mf::http::BufferInterface> /*buffer*/) override = 0;

    /**
     * @brief Called when an error occurs. Completes the request.
     *
     * @param[in] error_code The error code of the error.
     * @param[in] error_text Long description of the error.
     */
    virtual void RequestResponseErrorEvent(
            std::error_code /*error_code*/,
            std::string /*error_text*/) override = 0;

    /**
     * @brief Called when the request is successful. Completes the request.
     */
    virtual void RequestResponseCompleteEvent() override = 0;
};

}  // namespace downloader
}  // namespace mf
