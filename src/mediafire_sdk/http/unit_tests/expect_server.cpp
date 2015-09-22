/**
 * @file expect_server.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "expect_server.hpp"

using boost::logic::indeterminate;
using boost::asio::ip::tcp;
namespace asio = boost::asio;

ExpectServer::Pointer ExpectServer::Create(
        asio::io_service * io_service,
        std::shared_ptr<asio::io_service::work> work,
        uint16_t port
    )
{
    std::shared_ptr<ExpectServer> ptr(
            new ExpectServer(
                io_service,
                work,
                port
                )
            );

    ptr->CreateInit();

    return ptr;
}

ExpectServer::ExpectServer(boost::asio::io_service * io_service,
                           std::shared_ptr<boost::asio::io_service::work> work,
                           uint16_t port)
        : ExpectServerBase(io_service, work, port),
          acceptor_(*io_service, tcp::endpoint(tcp::v4(), port_)),
          socket_(*io_service)
{
}

void ExpectServer::CloseSocket() { socket_.close(); }

void ExpectServer::AsyncAccept()
{
    acceptor_.async_accept(socket_,
                           boost::bind(&ExpectServer::HandleAccept,
                                       shared_from_this(),
                                       asio::placeholders::error));
}

void ExpectServer::SendMessageWrite(
        const expect_server_test::SendMessage & node)
{
    // Without this the receiver might not get the message before timeout.
    boost::asio::ip::tcp::no_delay option(true);
    socket_.set_option(option);

    asio::async_write(socket_,
                      asio::buffer(*node.message),
                      boost::bind(&ExpectServer::HandleWrite,
                                  shared_from_this(),
                                  node.message,  // Pass to keep memory alive
                                  asio::placeholders::bytes_transferred,
                                  asio::placeholders::error));
}

void ExpectServer::ExpectRegexRead(const ExpectRegex & node)
{
    asio::async_read_until(
            socket_,
            read_buffer_,
            node.regex,
            boost::bind(
                &ExpectServer::HandleReadRegex,
                shared_from_this(),
                asio::placeholders::bytes_transferred,
                asio::placeholders::error
                )
            );
}

void ExpectServer::ExpectContentLengthRead(
        uint64_t bytes_to_read,
        const std::size_t total_bytes,
        std::size_t total_read_bytes
    )
{
    asio::async_read(
            socket_,
            read_buffer_,
            asio::transfer_exactly(static_cast<std::size_t>(bytes_to_read)),
            boost::bind(
                &ExpectServer::HandleReadContent,
                shared_from_this(),
                total_bytes,
                total_read_bytes,
                asio::placeholders::bytes_transferred,
                asio::placeholders::error
                )
            );
}

void ExpectServer::Handshake()
{
    // You will time out if you call this...
}
