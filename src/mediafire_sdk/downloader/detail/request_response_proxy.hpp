/**
 * @file mediafire_sdk/downloader/detail/request_response_proxy.hpp
 * @author Herbert Jones
 * @brief Wrapper to disable messages from an HttpRequest
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <memory>

#include "mediafire_sdk/http/request_response_interface.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

/**
 * @class RequestResponseProxy
 * @brief Wrapper to disable messages from an HttpRequest
 */
class RequestResponseProxy : public mf::http::RequestResponseInterface
{
public:
    RequestResponseProxy(
            std::shared_ptr<mf::http::RequestResponseInterface> target)
            : target_(target)
    {
    }

    virtual ~RequestResponseProxy() {}

    void Reset() { target_.reset(); }

private:
    virtual void RedirectHeaderReceived(const mf::http::Headers & headers,
                                        const mf::http::Url & new_url) override
    {
        if (target_)
            target_->RedirectHeaderReceived(headers, new_url);
    }

    virtual void ResponseHeaderReceived(
            const mf::http::Headers & headers) override
    {
        if (target_)
            target_->ResponseHeaderReceived(headers);
    }

    virtual void ResponseContentReceived(
            std::size_t start_pos,
            std::shared_ptr<mf::http::BufferInterface> buffer) override
    {

        if (target_)
            target_->ResponseContentReceived(start_pos, buffer);
    }

    virtual void RequestResponseErrorEvent(std::error_code error_code,
                                           std::string error_text) override
    {

        if (target_)
            target_->RequestResponseErrorEvent(error_code, error_text);
    }

    virtual void RequestResponseCompleteEvent() override
    {

        if (target_)
            target_->RequestResponseCompleteEvent();
    }

    std::shared_ptr<mf::http::RequestResponseInterface> target_;
};

}  // namespace detail
}  // namespace downloader
}  // namespace mf
