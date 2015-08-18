/**
 * @file race_preventer.hpp
 * @author Herbert Jones
 * @brief Class to prevent race conditions with socket communication and timer
 * timeouts.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "mediafire_sdk/http/detail/socket_wrapper.hpp"

namespace mf {
namespace http {
namespace detail {

/**
 * @class RacePreventer
 * @brief Prevent race conditions due to deadline timer asio issues.
 */
class RacePreventer
{
public:
    RacePreventer(SocketWrapper * wrapper) :
        first_taken_(std::make_shared<bool>(false)),
        ssl_socket_(wrapper->ssl_socket_),
        socket_(wrapper->socket_)
    {}

    bool IsFirst()
    {
        auto is_first = ! *first_taken_;
        *first_taken_ = true;
        return is_first;
    }

private:
    std::shared_ptr<bool> first_taken_;

    // SocketWrapper will release the socket on restart when a timeout occurs
    // and the async operation is cancelled.  Due to the asynchronous nature,
    // the socket read or write operation might still be on the io_service
    // function queue.  The socket needs to exist until the cancelled operation
    // calls its callback handler with operation_aborted.  We hold on to them
    // here, and when the cancelled callback completes, the race preventer
    // returns false on IsFirst, and then the socket will be destroyed when the
    // last RacePreventer is destroyed.
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> ssl_socket_;
    std::shared_ptr<asio::ip::tcp::socket> socket_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
