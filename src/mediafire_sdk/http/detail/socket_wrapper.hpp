/**
 * @file socket_wrapper.hpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

namespace mf
{
namespace http
{
namespace detail
{

namespace asio = boost::asio;

/**
 * @class SocketWrapper
 * @brief Wrapper to prevent deleting sockets that have pending operations.
 *
 * Cancelling and deleting a socket while the socket has ongoing operations does
 * not cancel the ongoing operations immediately.  Asio may still be writing to
 * the SSL "socket" with new data, and deleting that stream while it is in use
 * does cause memory access violations.  This class prevents a socket from being
 * deleted until we are absolutely sure the socket is no longer being used, and
 * informs the callback function for the asio operation that the operation is no
 * longer valid.
 */
class SocketWrapper
{
public:
    explicit SocketWrapper(
            asio::ssl::stream<asio::ip::tcp::socket> * socket
        )
    {
        ssl_socket_.reset(socket);
    }

    explicit SocketWrapper(
            asio::ip::tcp::socket * socket
        )
    {
        socket_.reset(socket);
    }

    /**
     * @brief DTOR
     *
     * We can destroy the socket now that all operations have ceased on it.
     */
    ~SocketWrapper()
    {
        if (ssl_socket_)
        {
            ssl_socket_.reset();
        }
        else if (socket_)
        {
            socket_.reset();
        }
    }

    /**
     * @brief Access the SSL socket.
     *
     * @return Pointer to the SSL socket
     */
    asio::ssl::stream<asio::ip::tcp::socket> * SslSocket()
    {
        return ssl_socket_.get();
    }

    /**
     * @brief Access the non-SSL socket.
     *
     * @return Pointer to the non-SSL socket
     */
    asio::ip::tcp::socket * Socket()
    {
        return socket_.get();
    }

    /**
     * @brief Cancel asynchronous operations on socket.
     */
    void Cancel()
    {
        if (ssl_socket_)
        {
            boost::system::error_code ec;
            ssl_socket_->lowest_layer().shutdown(
                    asio::ip::tcp::socket::shutdown_both, ec);
            ssl_socket_->lowest_layer().close(ec);
        }
        else if (socket_)
        {
            boost::system::error_code ec;
            socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket_->close(ec);
        }
    }

    asio::io_service & get_io_service()
    {
        if ( ssl_socket_ )
            return ssl_socket_->get_io_service();
        else
            return socket_->get_io_service();
    }

    template<typename ConstBuffer, typename ReadHandler>
    void async_read_some(ConstBuffer const_buffer, ReadHandler read_handler)
    {
        if ( ssl_socket_ )
            ssl_socket_->async_read_some(const_buffer, read_handler);
        else
            socket_->async_read_some(const_buffer, read_handler);
    }

    template<typename ConstBuffer, typename WriteHandler>
    void async_write_some(ConstBuffer const_buffer, WriteHandler write_handler)
    {
        if ( ssl_socket_ )
            ssl_socket_->async_write_some(const_buffer, write_handler);
        else
            socket_->async_write_some(const_buffer, write_handler);
    }

// private: -- clang on OSX 10.8 doesn't like friend class
    friend class RacePreventer;

    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> ssl_socket_;
    std::shared_ptr<asio::ip::tcp::socket> socket_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
