/**
 * @file expect_server_base.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "expect_server_base.hpp"

#include <iterator>
#include <map>
#include <string>

#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/get.hpp"

using boost::logic::indeterminate;
using boost::asio::ip::tcp;
namespace asio = boost::asio;

namespace {
    const uint32_t kInitExpectTimeoutMs = 600*1000;
    const uint32_t kExpectTimeoutMs = 600*1000;
    const uint64_t kMaxChunkRead = 1024*8;
}  // namespace

ExpectServerBase::ExpectServerBase(
        asio::io_service * io_service,
        std::shared_ptr<asio::io_service::work> work,
        uint16_t port
        ) :
    io_service_(io_service),
    port_(port),
    success_(indeterminate),
    disconnected_(false),
    headers_read_(false),
    timeout_ms_(kExpectTimeoutMs),
    work_(work),
    timer_(*io_service),
    total_read_(0)
{
}

void ExpectServerBase::CreateInit()
{
    // Open port synchronously.
    StartAccept();

    // Set timeout
    SetActionTimeout(kInitExpectTimeoutMs);
}

bool ExpectServerBase::Success()
{
    return (success_ == true);
}

std::error_code ExpectServerBase::Error()
{
    // if error
    if (error_code_)
        return error_code_;

    // May have unregistered error
    if ( ! pushed_errors_.empty() )
        return pushed_errors_.front();

    // No error
    return std::error_code();
}

void ExpectServerBase::Push(ExpectNode node)
{
    communications_.push_back(node);
}

void ExpectServerBase::RedirectHeaderReceived(
        const mf::http::Headers & /*headers*/,
        const mf::http::Url & /* new_url */
        )
{
    SetActionTimeout();
}

void ExpectServerBase::ResponseHeaderReceived(
        const mf::http::Headers & /* headers */
        )
{
    SetActionTimeout();

    // std::cout << "Headers read." << std::endl;

    headers_read_ = true;

    // Proceed if we were waiting for headers.
    if ( ! communications_.empty() )
    {
        ExpectNode & node = communications_.front();
        if ( boost::get<ExpectHeadersRead>(&node) )
        {
            communications_.pop_front();
            HandleNextCommunication();
        }
    }
}

void ExpectServerBase::ResponseContentReceived(
        std::size_t start_pos,
        std::shared_ptr<mf::http::BufferInterface> buffer
        )
{
    SetActionTimeout();

    // std::cout << "Client received content of " << buffer->Size()
    //     << " bytes starting from " << start_pos << std::endl;

    total_read_ += buffer->Size();
}

void ExpectServerBase::RequestResponseErrorEvent(
        std::error_code error_code,
        std::string error_text
        )
{
    SetActionTimeout();

    std::cout << "ERROR: Error received(" << error_code << "): "
        << error_text << std::endl;

    bool error_expected = false;
    bool do_next = false;

    if (!communications_.empty())
    {
        ExpectNode & top = communications_.front();
        if (ExpectError * exerr = boost::get<ExpectError>(&top))
        {
            if (exerr->error_condition == error_code)
            {
                std::cout << "Expected condition." << std::endl;

                error_expected = true;
                do_next = true;
            }
            else
            {
                std::cout << "Not expected condition." << std::endl;
            }
        }
    }

    if ( ! error_expected )
    {
        pushed_errors_.push_back(error_code);
    }

    if ( do_next )
    {
        communications_.pop_front();
        HandleNextCommunication();
    }
}

void ExpectServerBase::RequestResponseCompleteEvent()
{
    SetActionTimeout();

    // std::cout << "Completed." << std::endl;

    disconnected_ = true;

    // Alert if we were waiting for disconnect.
    if (!communications_.empty())
    {
        ExpectNode & top = communications_.front();
        if (ExpectDisconnect * ed = boost::get<ExpectDisconnect>(&top))
        {
            if (ed->total_bytes && *(ed->total_bytes) != total_read_)
            {
                std::cout << "ERROR: "
                             "Content does not match amount read." << std::endl;
                success_ = false;
                io_service_->stop();
            }
            else
            {
                communications_.pop_front();
                HandleNextCommunication();
            }
        }
        else if (boost::get<ExpectDisconnectAndReconnect>(&top))
        {
            std::cout << __FUNCTION__ << ": Got disconnect and reconnecting"
                      << std::endl;
            disconnected_ = false;
            communications_.pop_front();
            HandleNextCommunication();
        }
    }
}

void ExpectServerBase::operator()(const expect_server_test::SendMessage & node)
{
    // std::cout << "Server sending message of " << node.message->size()
    //           << " bytes." << std::endl;

    // std::cout << "==== START" << std::endl;
    // for (auto ch : *node.message)
    // {
    //     if (std::isprint(ch) || std::isspace(ch))
    //         std::cout << ch;
    //     else
    //         std::cout << std::hex << ch;
    // }
    // std::cout << "==== END" << std::endl;

    SendMessageWrite(node);
}

void ExpectServerBase::operator()(const ExpectRegex & node)
{
    // std::cout << "Expecting regex in message." << std::endl;

    ExpectRegexRead(node);
}

void ExpectServerBase::operator()(const ExpectError & err)
{
    // std::cout << "Expecting error:" << err.error_condition.message()
    //     << std::endl;

    if (!pushed_errors_.empty()
        && err.error_condition == pushed_errors_.front())
    {
        pushed_errors_.pop_front();
        communications_.pop_front();
        HandleNextCommunication();
    }
}

void ExpectServerBase::operator()(const ExpectContentLength & node)
{
    // std::cout << "Expecting content length of message: "
    //     << node.size << std::endl;

    // std::cout << "Existing buf size: " <<
    //     read_buffer_.size() << std::endl;

    uint64_t bytes_remaining = node.size;
    if (read_buffer_.size() > bytes_remaining)
        bytes_remaining = 0;
    else
        bytes_remaining -= read_buffer_.size();

    // std::cout << "Bytes remaining to read: " << bytes_remaining
    //     << std::endl;

    uint64_t bytes_to_read = std::min(kMaxChunkRead, bytes_remaining);

    std::size_t bytes_ready = read_buffer_.size();

    // std::cout << "Bytes waiting for: " << bytes_to_read << std::endl;

    ExpectContentLengthRead(bytes_to_read, node.size, bytes_ready);
}

void ExpectServerBase::operator()(const ExpectDisconnect & node)
{
    std::size_t expected_content_bytes = 0;
    if (node.total_bytes)
        expected_content_bytes = *node.total_bytes;

    // std::cout << "Client expecting disconnect after reading "
    //     << expected_content_bytes << " bytes of content." << std::endl;

    if (disconnected_)
    {
        if (node.total_bytes && *(node.total_bytes) != total_read_)
        {
            std::cout << "ERROR: Content does not match amount read."
                      << std::endl;
            success_ = false;
            io_service_->stop();
        }
        else
        {
            communications_.pop_front();
            HandleNextCommunication();
        }
    }
    // Else wait
}

void ExpectServerBase::operator()(const ExpectDisconnectAndReconnect &)
{
    // std::cout << "Expecting connect and disconnect" << std::endl;
    CloseSocket();

    // Open port synchronously.
    StartAccept();

    // // // Set timeout
    SetActionTimeout(1000 * 60);

    communications_.pop_front();
}

void ExpectServerBase::operator()(const ExpectHeadersRead &)
{
    // std::cout << "Expecting headers read." << std::endl;

    if (headers_read_)
    {
        // std::cout << "H: Headers read already." << std::endl;

        communications_.pop_front();
        HandleNextCommunication();
    }
    else
    {
        // std::cout << "H: Headers NOT read already." << std::endl;
    }
    // Else wait
}

void ExpectServerBase::operator()(const ExpectHandshake &)
{
    // std::cout << "Calling Handshake." << std::endl;

    Handshake();
}

void ExpectServerBase::StartAccept()
{
    AsyncAccept();
}

void ExpectServerBase::HandleAccept(const boost::system::error_code & err)
{

    if (!err)
    {
        if (communications_.empty())
        {
            std::cout << "ERROR: "
                         "Test failed due to no communications added."
                      << std::endl;
            io_service_->stop();
        }
        else
        {
            // std::cout << "Device connected." << std::endl;
            HandleNextCommunication();
        }
    }
    else
    {
        std::cout << __FUNCTION__ << ": Error: " << err.message() << std::endl;
    }
}

void ExpectServerBase::HandleNextCommunication()
{
    SetActionTimeout();

    if (communications_.empty())
    {
        // std::cout << "SERVER: At end." << std::endl;

        if (!pushed_errors_.empty())
        {
            error_code_ = pushed_errors_.front();
            success_ = false;
        }

        // Nothing left to do.
        if ( indeterminate(success_) )
            success_ = true;

        // Reset instead of stopping as there could be multiple servers.
        work_.reset();

        // Stop timer as it is considered "work" and we are done.
        timer_.cancel();

        // std::cout << "Expect Server completed normally." << std::endl;

        return;
    }

    ExpectNode & node = communications_.front();

    boost::apply_visitor( *this, node );
}

void ExpectServerBase::HandleWrite(
        std::shared_ptr<std::string> /* str_buffer */,
        const std::size_t written_bytes,
        const boost::system::error_code& err
    )
{
    if ( ! err )
    {
        if ( ! communications_.empty() )
        {
            ExpectNode & node = communications_.front();
            if (expect_server_test::SendMessage * sm
                = boost::get<expect_server_test::SendMessage>(&node))
            {
                assert( sm->message->size() == written_bytes );
                communications_.pop_front();
                HandleNextCommunication();
            }
        }
    }
}

void ExpectServerBase::HandleReadRegex(const std::size_t regex_bytes,
                                       const boost::system::error_code & err)
{
    if (!err)
    {
        // Prepare for next event.
        communications_.pop_front();

        // std::string data((std::istreambuf_iterator<char>(&read_buffer_)),
        //                  std::istreambuf_iterator<char>());
        // std::cout << data << std::endl;

        read_buffer_.consume(regex_bytes);

        HandleNextCommunication();
    }
}

void ExpectServerBase::HandleHandshake(const boost::system::error_code & err)
{
    if (!err)
    {
        // std::cout << "Got handshake." << std::endl;

        // Prepare for next event.
        communications_.pop_front();
        HandleNextCommunication();
    }
    else
    {
        std::cout << "Handshake failed:  " << err.message() << std::endl;
        success_ = false;
        io_service_->stop();
    }
}

void ExpectServerBase::HandleReadContent(
        const std::size_t total_bytes,
        std::size_t total_read_bytes,
        const std::size_t this_read_bytes,
        const boost::system::error_code& err
        )
{
    if ( ! err )
    {
        std::cout << "Got content." << std::endl;

        std::cout << "Total bytes: " << total_bytes << std::endl;
        std::cout << "Total read bytes: " << total_read_bytes << std::endl;
        std::cout << "Read this iteration: " << this_read_bytes << std::endl;

        total_read_bytes += this_read_bytes;
        if ( total_read_bytes == total_bytes )
        {
            read_buffer_.consume( total_read_bytes );

            // Success. Prepare for next event.
            communications_.pop_front();
            HandleNextCommunication();
        }
        else if ( total_read_bytes > total_bytes )
        {
            std::cout << "Total read bytes is unexpected!"
                " Total expected: " << total_bytes <<
                " Total received: " << total_read_bytes << std::endl;

            success_ = false;
        }
        else
        {
            // Continue on.
            uint64_t bytes_to_read = std::min(
                    kMaxChunkRead,
                    static_cast<uint64_t>(total_bytes) - total_read_bytes
                    );

            ExpectContentLengthRead(
                    bytes_to_read,
                    total_bytes,
                    total_read_bytes);
        }
    }
}

void ExpectServerBase::SetActionTimeout()
{
    SetActionTimeout(timeout_ms_);
}

void ExpectServerBase::SetActionTimeout(uint32_t timeout)
{
    timer_.expires_from_now(std::chrono::milliseconds(timeout));

    timer_.async_wait(boost::bind(&ExpectServerBase::ActionTimeout,
                                  shared_from_this(),
                                  asio::placeholders::error));
}

void ExpectServerBase::SetActionTimeoutMs(uint32_t timeout_ms)
{
    timeout_ms_ = timeout_ms;
}

void ExpectServerBase::ActionTimeout( const boost::system::error_code& err )
{
    if ( ! err )
    {
        std::cout << "ERROR: ExpectServer* action timeout reached."
            " Test failed." << std::endl;

        success_ = false;
        io_service_->stop();
    }
}
