/**
 * @file state_read_headers.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/find.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/types.hpp"
#include "mediafire_sdk/http/error.hpp"

namespace mf {
namespace http {
namespace detail {

char const * const header_terminator = "\r\n\r\n";

class ReadHeadersData
{
public:
    ReadHeadersData() :
        cancelled(false)
    {}

    bool cancelled;
};
using ReadHeadersDataPointer = std::shared_ptr<ReadHeadersData>;

template <typename FSM>
void HandleHeaderRead(
        FSM & fsm,
        ReadHeadersDataPointer state_data,
        SharedStreamBuf read_buffer,
        RacePreventer race_preventer,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (fsm.get_bw_analyser())
    {
        fsm.get_bw_analyser()->RecordIncomingBytes(
                read_buffer->size(), start_time, sclock::now());
    }

    HeadersReadEvent evt;
    {
        // Copy headers.

        asio::streambuf::const_buffers_type bufs = read_buffer->data();

        evt.raw_headers = std::string(
                asio::buffers_begin(bufs),
                (asio::buffers_begin(bufs) + bytes_transferred));
    }

    // Instead of checking for error, which is sometimes masked by SSL errors,
    // check for header termination.
    const bool headers_received = evt.raw_headers.find(header_terminator)
                                  != std::string::npos;

    if (headers_received)
    {
        // Pass the buffer incase it contains any unread data.
        evt.read_buffer = read_buffer;

        std::istream response_stream(read_buffer.get());
        response_stream >> evt.http_version;
        response_stream >> evt.status_code;
        std::getline(response_stream, evt.status_message);

        if ( ! response_stream
                || evt.http_version.substr(0, 5) != "HTTP/")
        {
            std::stringstream ss;
            ss << "Protocol error while parsing headers("
                << fsm.get_url() << ").";
            fsm.ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        http_error::UnparsableHeaders ),
                    ss.str()
                });
            return;
        }

        std::string line;
        std::string last_header_name;
        while (std::getline(response_stream, line) && line != "\r")
        {
            boost::trim_right(line);

            if ( line.empty() ) continue;

            // HTTP headers can be split up inbetween lines.
            // http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2
            if ( line[0] == ' ' || line[0] == '\t' )
            {
                if ( last_header_name.empty() )
                {
                    std::stringstream ss;
                    ss << "Failure while reading headers url("
                        << fsm.get_url() << ").";
                    ss << " Badly formatted headers.";
                    fsm.ProcessEvent(
                        ErrorEvent{
                            make_error_code(
                                http_error::UnparsableHeaders ),
                            ss.str()
                        });
                    return;
                }

                // This is a continuation of previous line.
                auto it = evt.headers.find(last_header_name);
                it->second += " ";
                boost::trim(line);
                it->second += line;

                continue;
            }

            boost::iterator_range<std::string::iterator> result =
                boost::find_first(line, ":");

            if ( ! result.empty() )
            {
                std::string header_name = std::string(
                        line.begin(), result.begin());

                std::string header_value = std::string(
                        result.end(), line.end());

                boost::trim(header_name);
                boost::to_lower(header_name);

                boost::trim(header_value);

                evt.headers.emplace(header_name, header_value);

                // Record the last header name in case the next line is
                // extended.
                last_header_name.swap(header_name);
            }
        }

        {
            auto it = evt.headers.find("content-length");
            if ( it != evt.headers.end() )
            {
                try {
                    evt.content_length = boost::lexical_cast<uint64_t>(
                            it->second);
                } catch(boost::bad_lexical_cast &) {
                    std::stringstream ss;
                    ss << "Failure while parsing headers url("
                        << fsm.get_url() << ").";
                    ss << " Invalid Content-Length: " << it->second;
                    fsm.ProcessEvent(
                        ErrorEvent{
                            make_error_code(
                                http_error::UnparsableHeaders ),
                            ss.str()
                        });
                    return;
                }
            }
        }

        fsm.ProcessEvent(evt);  // Send HeadersReadEvent
    }
    else if (!err)
    {
        std::stringstream ss;
        ss << "Failure while reading headers url(" << fsm.get_url() << ").";
        ss << " Header termination not in response.";
        ss << " Received: \"" << evt.raw_headers << "\"";
        fsm.ProcessEvent(
                ErrorEvent{make_error_code(http_error::ReadFailure), ss.str()});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while reading headers url(" << fsm.get_url() << ").";
        ss << " Error: (" << err.message() << ")";
        ss << " Received: \"" << evt.raw_headers << "\"";
        fsm.ProcessEvent(
                ErrorEvent{make_error_code(http_error::ReadFailure), ss.str()});
    }
}

class ReadHeaders : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const &, FSM & fsm)
    {
        auto state_data = std::make_shared<ReadHeadersData>();
        state_data_ = state_data;

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("read response header",
            fsm.get_timeout_seconds());
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        auto read_buffer = std::make_shared<boost::asio::streambuf>();

        asio::async_read_until(
                *fsm.get_socket_wrapper(), *read_buffer, header_terminator,
                [fsmp, state_data, read_buffer, race_preventer, start_time](
                        const boost::system::error_code & ec,
                        std::size_t bytes_transferred)
                {
                    HandleHeaderRead(*fsmp, state_data, read_buffer,
                                     race_preventer, start_time,
                                     bytes_transferred, ec);
                });
    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    ReadHeadersDataPointer state_data_;
};


}  // namespace detail
}  // namespace http
}  // namespace mf
