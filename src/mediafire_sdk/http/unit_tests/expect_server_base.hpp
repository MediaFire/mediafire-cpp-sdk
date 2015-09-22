/**
 * @file expect_server_base.hpp
 * @author Herbert Jones
 * @brief Expect server. Base for http and https.
 *
 * @copyright Copyright 2014 Mediafire
 *
 * The expect server handles both sides of an HTPP request, for testing
 * purposes, so that we can test various HttpRequest and Api calls.
 */
#pragma once

#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <system_error>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/bind.hpp"
#include "boost/logic/tribool.hpp"
#include "boost/optional.hpp"
#include "boost/random/random_device.hpp"
#include "boost/random/uniform_int_distribution.hpp"
#include "boost/regex.hpp"
#include "boost/variant/variant.hpp"

#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/headers.hpp"
namespace expect_server_test
{
    struct SendMessage
    {
        explicit SendMessage(std::string str) :
            message(std::make_shared<std::string>(std::move(str)))
        {}

        std::shared_ptr<std::string> message;
    };
}  // namespace expect_server_test

struct ExpectRegex
{
    boost::regex regex;
};
struct ExpectError
{
    std::error_code error_condition;
};
struct ExpectPost
{
    std::size_t size;
};
struct ExpectHeadersRead {};
struct ExpectRedirect {};
struct ExpectContentLength
{
    std::size_t size;
};
struct ExpectDisconnect
{
    boost::optional<std::size_t> total_bytes;
};
struct ExpectDisconnectAndReconnect
{
};
struct ExpectHandshake
{
};

using ExpectNode = boost::variant<expect_server_test::SendMessage,
                                  ExpectRegex,
                                  ExpectError,
                                  ExpectContentLength,
                                  ExpectHeadersRead,
                                  ExpectDisconnect,
                                  ExpectDisconnectAndReconnect,
                                  ExpectHandshake>;

/**
 * The expect server handles both sides of an HTPP request, for testing
 * purposes, so that we can test various HttpRequest and Api calls.
 */
class ExpectServerBase :
    public std::enable_shared_from_this<ExpectServerBase>,
    public mf::http::RequestResponseInterface,
    public boost::static_visitor<>
{
public:
    bool Success();

    std::error_code Error();

    void Push(ExpectNode node);

    // --- Overrides for mf::http::RequestResponseInterface ---
    virtual void RedirectHeaderReceived(
            const mf::http::Headers & /* headers */,
            const mf::http::Url & /* new_url */
            ) override;

    virtual void ResponseHeaderReceived(
            const mf::http::Headers & headers
            ) override;

    virtual void ResponseContentReceived(
            std::size_t start_pos,
            std::shared_ptr<mf::http::BufferInterface> buffer
            ) override;

    virtual void RequestResponseErrorEvent(
            std::error_code error_code,
            std::string error_text
            ) override;

    virtual void RequestResponseCompleteEvent() override;

    // Apply visitors
    void operator()(const expect_server_test::SendMessage &);
    void operator()(const ExpectRegex &);
    void operator()(const ExpectError &);
    void operator()(const ExpectContentLength &);
    void operator()(const ExpectDisconnect &);
    void operator()(const ExpectDisconnectAndReconnect &);
    void operator()(const ExpectHeadersRead &);
    void operator()(const ExpectHandshake &);

    virtual void SetActionTimeoutMs(uint32_t timeout_ms);

protected:
    boost::asio::io_service * io_service_;
    uint16_t port_;
    boost::logic::tribool success_;
    std::error_code error_code_;
    bool disconnected_;
    bool headers_read_;

    uint32_t timeout_ms_;

    std::shared_ptr<boost::asio::io_service::work> work_;
    boost::asio::steady_timer timer_;

    std::list<ExpectNode> communications_;
    std::list<std::error_code> pushed_errors_;

    boost::asio::streambuf read_buffer_;

    uint64_t total_read_;

    ExpectServerBase(
            boost::asio::io_service * io_service,
            std::shared_ptr<boost::asio::io_service::work> work,
            uint16_t port
            );

    void CreateInit();

    void StartAccept();

    void HandleAccept( const boost::system::error_code& err );

    void HandleNextCommunication();

    void HandleWrite(
            std::shared_ptr<std::string> str_buffer,
            const std::size_t written_bytes,
            const boost::system::error_code& err
            );

    void HandleReadRegex(
            const std::size_t regex_bytes,
            const boost::system::error_code& err
            );

    void HandleReadContent(
            const std::size_t total_bytes,
            std::size_t total_read_bytes,
            const std::size_t this_read_bytes,
            const boost::system::error_code& err
            );

    void HandleHandshake(
            const boost::system::error_code& err
            );

    void SetActionTimeout();

    void SetActionTimeout(uint32_t timeout);

    void ActionTimeout(const boost::system::error_code & err);

    // Async overrides
    virtual void CloseSocket() = 0;
    virtual void AsyncAccept() = 0;
    virtual void Handshake() = 0;
    virtual void SendMessageWrite(const expect_server_test::SendMessage & node)
            = 0;
    virtual void ExpectRegexRead(const ExpectRegex & node) = 0;
    virtual void ExpectContentLengthRead(uint64_t bytes_to_read,
                                         const std::size_t total_bytes,
                                         std::size_t total_read_bytes) = 0;
};
