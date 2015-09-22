/**
 * @file expect_server.hpp
 * @author Herbert Jones
 * @brief Expect server http version.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "mediafire_sdk/http/unit_tests/expect_server_base.hpp"

class ExpectServer :
    public ExpectServerBase
{
public:
    typedef std::shared_ptr<ExpectServer> Pointer;

    static Pointer Create(
            boost::asio::io_service * io_service,
            std::shared_ptr<boost::asio::io_service::work> work,
            uint16_t port
            );

private:
    boost::asio::ip::tcp::acceptor acceptor_;

    boost::asio::ip::tcp::socket socket_;

    // boost::asio::ssl::context context_;

    ExpectServer(boost::asio::io_service * io_service,
                 std::shared_ptr<boost::asio::io_service::work> work,
                 uint16_t port);

    // Async overrides
    virtual void CloseSocket() override;
    virtual void AsyncAccept() override;
    virtual void Handshake() override;
    virtual void SendMessageWrite(
            const expect_server_test::SendMessage & node) override;
    virtual void ExpectRegexRead(const ExpectRegex & node) override;
    virtual void ExpectContentLengthRead(
            uint64_t bytes_to_read,
            const std::size_t total_bytes,
            std::size_t total_read_bytes
        ) override;
};
