/**
 * @file state_read_content.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/restrict.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/encoding.hpp"
#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/detail/types.hpp"
#include "mediafire_sdk/http/error.hpp"

#include "mediafire_sdk/utils/string.hpp"

namespace {
static const uint64_t kMaxUnknownReadLength = 1024 * 8;

bool IsSslShortRead(const boost::system::error_code & ec)
{
    // Asio internally uses OpenSSL ERR_PACK, which adds library code, function
    // code and reason.  The reason is in the lowest 3 hexadecimal digits.
    bool ret = (ec.category() == boost::asio::error::get_ssl_category()
        && (ec.value() & 0xfff) == SSL_R_SHORT_READ);

    return ret;
}

int ParseTransferEncoding(
        const std::map<std::string, std::string> & headers
    )
{
    using namespace mf::http::detail;

    // See http://www.iana.org/assignments/http-parameters/http-parameters.xhtml

    int ret = static_cast<int>(TE_None);

    auto it = headers.find("content-length");
    if ( it != headers.end() )
        ret |= static_cast<int>(TE_ContentLength);

    it = headers.find("transfer-encoding");
    if ( it != headers.end() )
    {
        std::vector<std::string> tokens;
        boost::split(tokens, it->second, boost::is_any_of(","));
        std::for_each(std::begin(tokens), std::end(tokens),
                [](std::string & v){
                    boost::trim(v);
                } );

        for (const std::string & token : tokens)
        {
            if ( token == "chunked" )
                ret |= static_cast<int>(TE_Chunked);
            else if ( token == "gzip" )
                ret |= static_cast<int>(TE_Gzip);
            else
                ret |= static_cast<int>(TE_Unknown);
        }
    }

    return ret;
}

int ParseContentEncoding(
        const std::map<std::string, std::string> & headers
    )
{
    using namespace mf::http::detail;

    int ret = static_cast<int>(CE_None);

    auto it = headers.find("content-encoding");

    if ( it != headers.end() )
    {
        std::vector<std::string> tokens;
        boost::split(tokens, it->second, boost::is_any_of(","));
        std::for_each(std::begin(tokens), std::end(tokens),
                [](std::string & v){
                    boost::trim(v);
                } );

        for (const std::string & token : tokens)
        {
            if ( token == "gzip" )
                ret |= static_cast<int>(CE_Gzip);
            else
                ret |= static_cast<int>(CE_Unknown);
        }
    }

    return ret;
}

class HttpRequestBuffer : public mf::http::BufferInterface
{
public:
    HttpRequestBuffer(std::unique_ptr<uint8_t[]> buffer, uint64_t size) :
        buffer_(std::move(buffer)),
        size_(size)
    {}
    virtual ~HttpRequestBuffer() {}

    virtual uint64_t Size() const
    {
        return size_;
    }

    /**
     * @brief Get buffer
     *
     * @return pointer that can be read with data size contained in Size().
     */
    virtual const uint8_t * Data() const
    {
        return buffer_.get();
    }
private:
    std::unique_ptr<uint8_t[]> buffer_;
    uint64_t size_;
};

class VectorBuffer : public mf::http::BufferInterface
{
public:
    VectorBuffer() {}
    virtual ~VectorBuffer() {}

    virtual uint64_t Size() const
    {
        return buffer.size();
    }

    /**
     * @brief Get buffer
     *
     * @return pointer that can be read with data size contained in Size().
     */
    virtual const uint8_t * Data() const
    {
        return buffer.data();
    }

    std::vector<uint8_t> buffer;
private:
};
}  // namespace

namespace mf {
namespace http {
namespace detail {

/**
 * @struct ReadContentData
 * @brief Data specific to the read content state.
 */
struct ReadContentData
{
    ReadContentData() :
        cancelled(false),
        content_length(0),
        filter_buf_consumed(0),
        using_gzip(false),
        using_content_length(false)
    {}

    bool cancelled;

    uint64_t content_length;

    SharedStreamBuf read_buffer;

    asio::streambuf gzipped_buffer;
    std::size_t filter_buf_consumed;

    bool using_gzip;
    bool using_content_length;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> filter_buf;
};
using ReadContentDataPointer = std::shared_ptr<ReadContentData>;

// -- Forward declarations -----------------------------------------------------
template <typename FSM>
void HandleContentRead(
        FSM & fsm,
        ReadContentDataPointer state_data,
        RacePreventer race_preventer,
        const std::size_t total_previously_read,
        const TimePoint start_time,
        const std::size_t bytes_previously_transferred,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    );
template <typename FSM>
void HandleContentReadDelayCallback(
        FSM & fsm,
        ReadContentDataPointer state_data,
        const uint64_t max_read_size,
        const std::size_t total_read
    );
template <typename FSM>
void HandleContentChunkRead(
        FSM & fsm,
        ReadContentDataPointer state_data,
        RacePreventer race_preventer,
        uint64_t output_bytes_consumed,  // What we have sent to user.
        uint64_t chunk_size,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    );
template <typename FSM>
void HandleContentChunkSizeReadDelay(
        FSM & fsm,
        ReadContentDataPointer state_data,
        uint64_t left_to_read,
        uint64_t output_bytes_consumed,
        uint64_t chunk_size
    );
template <typename FSM>
void HandleNextChunk(
        FSM & fsm,
        ReadContentDataPointer state_data,
        uint64_t output_bytes_consumed,  // What we have sent to user.
        const TimePoint start_time,
        Duration content_chunk_read_duration,
        uint64_t chunk_size
    );
template <typename FSM>
void HandleCompleteAllChunks(
        FSM & fsm,
        ReadContentDataPointer state_data
    );
template <typename FSM>
void HandleContentChunkSizeRead(
        FSM & fsm,
        ReadContentDataPointer state_data,
        RacePreventer race_preventer,
        uint64_t output_bytes_consumed,  // What we have sent to user.
        const TimePoint start_time,
        Duration content_chunk_read_duration,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    );
// -- END Forward declarations -------------------------------------------------


template <typename FSM>
void HandleContentRead(
        FSM & fsm,
        ReadContentDataPointer state_data,
        RacePreventer race_preventer,
        const std::size_t total_previously_read,
        const TimePoint start_time,
        const std::size_t bytes_previously_transferred,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    if (state_data->cancelled == true)
        return;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    const std::size_t bytes_to_process =
        bytes_previously_transferred + bytes_transferred;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (fsm.get_bw_analyser())
    {
        fsm.get_bw_analyser()->RecordIncomingBytes(
            bytes_previously_transferred, start_time, sclock::now() );
    }

    const std::size_t total_read = total_previously_read + bytes_to_process;

    const bool connection_terminated = static_cast<bool>(err);
    const bool connection_terminated_cleanly = (err == asio::error::eof);

    const bool using_content_length
        = state_data->using_content_length;

    const bool reached_full_content
            = using_content_length && state_data->content_length == total_read;

    const bool exceeded_full_content
            = using_content_length && state_data->content_length < total_read;

    // Fail if content length incorrect on read complete
    if (exceeded_full_content)
    {
        // We have read more data than we requested, and more than the server
        // told us to expect.
        std::stringstream ss;
        ss << "Failure while reading content.";
        ss << " Url: " << fsm.get_url();
        ss << " Error: Exceeded content length.";
        ss << " Total read: " << total_read;
        ss << " Content length: " << state_data->content_length;

        fsm.ProcessEvent(
                ErrorEvent{make_error_code(http_error::ReadFailure), ss.str()});

        assert(!"Content length exceeded in HandleContentRead.");
    }
    else if (connection_terminated && using_content_length
             && !reached_full_content)
    {
        // The connection was terminated and we know we haven't read enough
        // data yet.
        std::stringstream ss;
        ss << "Early termination of content read.";
        ss << " Url: " << fsm.get_url();
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
                ErrorEvent{make_error_code(http_error::ReadFailure), ss.str()});
    }
    else if (connection_terminated && !connection_terminated_cleanly
             && !using_content_length)
    {
        // If the connection wasn't terminated cleanly and we don't have the
        // content length to rely on, then the transaction failed.
        std::stringstream ss;
        ss << "Connection failure.";
        ss << " Url: " << fsm.get_url();
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
                ErrorEvent{make_error_code(http_error::ReadFailure), ss.str()});
    }
    else
    {
        // Either we are done reading, or have more to read at this point.

        // Send data to client immediately if not using gzip.
        if (!state_data->using_gzip)
        {
            // Non gzip buffer passing.
            std::istream post_data_stream(state_data->read_buffer.get());
            std::unique_ptr<uint8_t[]> data(new uint8_t[bytes_to_process]);
            post_data_stream.read( reinterpret_cast<char*>(data.get()),
                bytes_to_process );
            std::shared_ptr<mf::http::BufferInterface> return_buffer(
                    new HttpRequestBuffer(
                        std::move(data),
                        bytes_to_process )
                    );

            auto iface = fsm.get_callback();

            fsm.get_callback_io_service()->dispatch(
                    [iface, return_buffer, total_previously_read]()
                    {
                        iface->ResponseContentReceived(
                            total_previously_read, return_buffer );
                    }
                );
        }

        if (!connection_terminated && !reached_full_content)
        {
            // There is more content to receive.

            uint64_t max_read_size = kMaxUnknownReadLength;
            if (state_data->using_content_length)
            {
                max_read_size
                        = std::min(state_data->content_length - total_read,
                                   kMaxUnknownReadLength);
            }

            auto fsmp = fsm.AsFrontShared();

            // Delay behavior:
            fsm.SetTransactionDelayTimer(start_time, sclock::now());

            fsm.get_transmission_delay_timer()->async_wait(
                    fsm.get_event_strand()->wrap(
                            [fsmp, state_data, max_read_size, total_read](
                                    const boost::system::error_code & ec)
                            {
                                fsmp->SetTransactionDelayTimerWrapper(
                                        [fsmp, state_data, max_read_size,
                                         total_read]()
                                        {
                                            HandleContentReadDelayCallback(
                                                    *fsmp, state_data,
                                                    max_read_size, total_read);
                                        },
                                        ec);
                            }));
        }
        else
        {
            // We have all the content.
            assert(connection_terminated || reached_full_content);

            // Unzip and send all data if using gzip now.
            if ( state_data->using_gzip )
            {
                auto & filter_buffer = state_data->filter_buf;

                filter_buffer.push( *state_data->read_buffer );

                std::shared_ptr<VectorBuffer> return_buffer(
                        new VectorBuffer() );

                try {
                    boost::iostreams::copy(
                            filter_buffer,
                            std::back_inserter(return_buffer->buffer));
                }
                catch( std::exception & err )
                {
                    std::stringstream ss;
                    ss << "Compression failure.";
                    ss << " Error: " << err.what();
                    fsm.ProcessEvent(
                        ErrorEvent{
                            make_error_code(
                                http_error::ReadFailure ),
                            ss.str()
                        });
                    return;
                }

                auto iface = fsm.get_callback();
                const uint64_t start_pos = state_data->filter_buf_consumed;

                fsm.get_callback_io_service()->dispatch(
                        [iface, return_buffer, start_pos]()
                        {
                            iface->ResponseContentReceived(
                                start_pos, return_buffer );
                        }
                    );

                // New byte counts
                state_data->filter_buf_consumed += return_buffer->buffer.size();
            }

            // We are done receiving data from the remote source.
            fsm.ProcessEvent(ContentReadEvent{});
        }
    }
}

template <typename FSM>
void HandleContentReadDelayCallback(
        FSM & fsm,
        ReadContentDataPointer state_data,
        const uint64_t max_read_size,
        const std::size_t total_read
    )
{
    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Ensure a cancellation doesn't mess up the state due to async
    // timer.
    if ( fsm.get_transmission_delay_timer_enabled() )
    {
        fsm.set_transmission_delay_timer_enabled(false);

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("read response content 5",
            fsm.get_timeout_seconds());
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        asio::async_read(*fsm.get_socket_wrapper(), *state_data->read_buffer,
            asio::transfer_exactly(max_read_size),
            [fsmp, state_data, race_preventer, total_read, start_time](
                    const boost::system::error_code& ec,
                    std::size_t bytes_transferred
                )
            {
                HandleContentRead(*fsmp, state_data, race_preventer, total_read,
                    start_time, 0, bytes_transferred, ec);
            });
    }
}

template <typename FSM>
void HandleContentChunkRead(
        FSM & fsm,
        ReadContentDataPointer state_data,
        RacePreventer race_preventer,
        uint64_t output_bytes_consumed,  // What we have sent to user.
        uint64_t chunk_size,
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

    if (auto bwa = fsm.get_bw_analyser())
    {
        bwa->RecordIncomingBytes( bytes_transferred, start_time,
            sclock::now() );
    }

    auto content_chunk_read_duration = AsDuration(sclock::now() -
        start_time);
    bool eof = false;

    if ( err == asio::error::eof )
    {
        eof = true;
    }
    else if ( fsm.get_is_ssl() && IsSslShortRead(err) )
    {
        // SSL doesn't return EOF when it ends.
        eof = true;
    }

    if ( !err || eof )
    {
        if ( state_data->using_gzip )
        {
            boost::iostreams::copy(
                    boost::iostreams::restrict(
                        *state_data->read_buffer, 0, chunk_size ),
                    state_data->gzipped_buffer );

            output_bytes_consumed += chunk_size;
        }
        else
        {
            // Non gzip buffer passing.
            std::istream post_data_stream(state_data->read_buffer.get());
            std::unique_ptr<uint8_t[]> data( new uint8_t[chunk_size] );
            post_data_stream.read( reinterpret_cast<char*>(data.get()),
                chunk_size );

            std::shared_ptr<mf::http::BufferInterface> return_buffer(
                    new HttpRequestBuffer(
                        std::move(data),
                        chunk_size )
                    );
            auto iface = fsm.get_callback();
            const auto start_pos = output_bytes_consumed;
            fsm.get_callback_io_service()->dispatch(
                    [iface, return_buffer, start_pos]()
                    {
                        iface->ResponseContentReceived(
                            start_pos, return_buffer );
                    }
                    );

            // New byte counts
            output_bytes_consumed += chunk_size;
        }

        state_data->read_buffer->consume( 2 );  // +2 for \r\n after chunk.

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("read response content 4",
            fsm.get_timeout_seconds());
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        asio::async_read_until(
            *fsm.get_socket_wrapper(),
            *state_data->read_buffer,
            "\r\n",
            [fsmp, state_data, race_preventer, start_time,
                output_bytes_consumed, content_chunk_read_duration](
                    const boost::system::error_code& ec,
                    std::size_t bytes_transferred
                )
            {
                HandleContentChunkSizeRead(*fsmp, state_data, race_preventer,
                    output_bytes_consumed, start_time,
                    content_chunk_read_duration, bytes_transferred, ec);
            });
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while reading chunked content.";
        ss << " Url: " << fsm.get_url();
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::ReadFailure ),
                ss.str()
            });
    }
}

template <typename FSM>
void HandleContentChunkSizeReadDelay(
        FSM & fsm,
        ReadContentDataPointer state_data,
        uint64_t left_to_read,
        uint64_t output_bytes_consumed,
        uint64_t chunk_size
    )
{
    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Ensure a cancellation doesn't mess up the state due to async
    // timer.
    if ( fsm.get_transmission_delay_timer_enabled() )
    {
        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("read response content 3",
            fsm.get_timeout_seconds());
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        // transfer_exactly required as we want the whole chunk
        asio::async_read(*fsm.get_socket_wrapper(), *state_data->read_buffer,
            asio::transfer_exactly(left_to_read),
            [fsmp, state_data, race_preventer, output_bytes_consumed,
                chunk_size, start_time](
                    const boost::system::error_code& ec,
                    std::size_t bytes_transferred
                )
            {
                HandleContentChunkRead(*fsmp, state_data, race_preventer,
                    output_bytes_consumed, chunk_size, start_time,
                    bytes_transferred, ec);
            });
    }
}

template <typename FSM>
void HandleNextChunk(
        FSM & fsm,
        ReadContentDataPointer state_data,
        uint64_t output_bytes_consumed,  // What we have sent to user.
        const TimePoint start_time,
        Duration content_chunk_read_duration,
        uint64_t chunk_size
    )
{
    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    TimePoint now = sclock::now();

    // Determine how much more to read, or if the buffer
    // already contains enough data to continue.
    uint64_t left_to_read = chunk_size+2;
    if ( state_data->read_buffer->size() > left_to_read )
        left_to_read = 0;
    else
        left_to_read -= state_data->read_buffer->size();

    // Delay behavior:
    auto total_duration = AsDuration(content_chunk_read_duration +
        (now - start_time));
    fsm.SetTransactionDelayTimer( now, total_duration );

    auto fsmp = fsm.AsFrontShared();

    fsm.get_transmission_delay_timer()->async_wait(
        fsm.get_event_strand()->wrap(
            [fsmp, state_data, left_to_read, output_bytes_consumed, chunk_size](
                    const boost::system::error_code& ec
                )
            {
                fsmp->SetTransactionDelayTimerWrapper(
                    [fsmp, state_data, left_to_read, output_bytes_consumed,
                        chunk_size]()
                    {
                        HandleContentChunkSizeReadDelay(*fsmp, state_data,
                            left_to_read, output_bytes_consumed, chunk_size);
                    },
                    ec);
            }));
}

template <typename FSM>
void HandleCompleteAllChunks(
        FSM & fsm,
        ReadContentDataPointer state_data
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    if ( state_data->using_gzip )
    {
        std::shared_ptr<VectorBuffer> return_buffer(
            new VectorBuffer() );

        auto & filter_buffer = state_data->filter_buf;

        filter_buffer.push( state_data->gzipped_buffer );

        try {
            boost::iostreams::copy(
                filter_buffer,
                std::back_inserter(return_buffer->buffer));
        }
        catch( boost::iostreams::gzip_error & err )
        {
            std::stringstream ss;
            ss << "Compression failure.";
            ss << " Error: " << err.what();
            switch (err.error())
            {
                case boost::iostreams::gzip::zlib_error:
                    ss << " GZip error: zlib_error";
                    break;
                case boost::iostreams::gzip::bad_crc:
                    ss << " GZip error: bad_crc";
                    break;
                case boost::iostreams::gzip::bad_length:
                    ss << " GZip error: bad_length";
                    break;
                case boost::iostreams::gzip::bad_header:
                    ss << " GZip error: bad_header";
                    break;
                case boost::iostreams::gzip::bad_footer:
                    ss << " GZip error: bad_footer";
                    break;
                default:
                    ss << " GZip error: " << err.error();
                    break;
            }
            ss << " Zlib error: " << err.zlib_error_code();
            fsm.ProcessEvent(ErrorEvent{
                make_error_code(
                    http_error::CompressionFailure ),
                ss.str()
                });
            return;
        }
        catch( std::exception & err )
        {
            std::stringstream ss;
            ss << "Compression failure.";
            ss << " Error: " << err.what();
            fsm.ProcessEvent(ErrorEvent{
                make_error_code(
                    http_error::CompressionFailure ),
                ss.str()
                });
            //assert(!"GZip compression error from chunks");
            return;
        }

        auto iface = fsm.get_callback();

        const uint64_t start_pos = state_data->filter_buf_consumed;
        fsm.get_callback_io_service()->dispatch(
            [iface, return_buffer, start_pos]()
            {
                iface->ResponseContentReceived(
                    start_pos, return_buffer );
            }
        );

        // New byte counts
        state_data->filter_buf_consumed += return_buffer->buffer.size();
    }

    fsm.ProcessEvent(ContentReadEvent{});
}

template <typename FSM>
void HandleContentChunkSizeRead(
        FSM & fsm,
        ReadContentDataPointer state_data,
        RacePreventer race_preventer,
        uint64_t output_bytes_consumed,  // What we have sent to user.
        const TimePoint start_time,
        Duration content_chunk_read_duration,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;
    using sclock = std::chrono::steady_clock;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    TimePoint now = sclock::now();
    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (fsm.get_bw_analyser())
    {
        fsm.get_bw_analyser()->RecordIncomingBytes( bytes_transferred,
            start_time, now );
    }

    if ( !err || err == asio::error::eof )
    {
        std::string chunk_size_as_hex;
        std::istream response_stream(state_data->read_buffer.get());
        std::getline(response_stream, chunk_size_as_hex);

        uint64_t chunk_size = 0;
        try
        {
            chunk_size = mf::utils::str_to_uint64(chunk_size_as_hex, 16);
        }
        catch(std::invalid_argument & err)
        {
            std::stringstream ss;
            ss << "Failure while parsing chunk: " << err.what();
            ss << " Chunk: " << chunk_size_as_hex;
            fsm.ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        http_error::ReadFailure ),
                    ss.str()
                });
            return;
        }
        catch(std::out_of_range & err)
        {
            std::stringstream ss;
            ss << "Failure while parsing chunk: " << err.what();
            ss << " Chunk: " << chunk_size_as_hex;
            fsm.ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        http_error::ReadFailure ),
                    ss.str()
                });
            return;
        }

        if ( chunk_size == 0 )  // Last chunk is 0 bytes long.
        {
            HandleCompleteAllChunks(fsm, state_data);
        }
        else
        {
            HandleNextChunk( fsm, state_data, output_bytes_consumed, start_time,
                content_chunk_read_duration, chunk_size);
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while reading content.";
        ss << " Url: " << fsm.get_url();
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::ReadFailure ),
                ss.str()
            });
    }
}

class ReadContent : public boost::msm::front::state<>
{
public:
    template <typename FSM>
    void on_entry(HeadersParsedEvent const & evt, FSM & fsm)
    {
        using mf::http::http_error;

        auto state_data = std::make_shared<ReadContentData>();
        state_data_ = state_data;

        const auto & headers = evt.headers;

        // Encodings!
        const int transfer_encoding = ParseTransferEncoding(headers);
        const int content_encoding = ParseContentEncoding(headers);

        // gzip can be in transfer-encoding or content-encoding...
        if (transfer_encoding & TE_Gzip || content_encoding & CE_Gzip)
            state_data->using_gzip = true;

        if ( transfer_encoding & TE_ContentLength )
            state_data->using_content_length = true;

        if ( transfer_encoding & TE_Unknown )
        {
            std::stringstream ss;
            ss << "Unsupported transfer-encoding.";
            auto it = headers.find("transfer-encoding");
            if ( it != headers.end() )
                ss << " Transfer-Encoding: " << it->second;
            fsm.ProcessEvent(ErrorEvent{
                    make_error_code(
                        http_error::UnsupportedEncoding ),
                    ss.str()
                });
            return;
        }

        if ( content_encoding & CE_Unknown )
        {
            std::stringstream ss;
            ss << "Unsupported content-encoding.";
            auto it = headers.find("content-encoding");
            if ( it != headers.end() )
                ss << " Content-Encoding: " << it->second;
            fsm.ProcessEvent(ErrorEvent{
                    make_error_code(
                        http_error::UnsupportedEncoding ),
                    ss.str()
                });
            return;
        }

        if ( transfer_encoding & TE_Chunked
            && state_data->using_content_length )
        {
            std::stringstream ss;
            ss << "Unable to handle chunked encoding and content length.";
            ss << " Violates RFC 2616, Section 4.4";
            auto it = headers.find("content-encoding");
            if ( it != headers.end() )
                ss << " Content-Encoding: " << it->second;
            it = headers.find("content-length");
            if ( it != headers.end() )
                ss << " Content-Length: " << it->second;
            fsm.ProcessEvent(ErrorEvent{
                    make_error_code(
                        http_error::UnsupportedEncoding ),
                    ss.str()
                });
            return;
        }

        // Populate state specific data
        state_data->content_length = evt.content_length;

        // Move any previouly gathered data from the header reading over to the
        // read buffer.
        state_data->read_buffer = evt.read_buffer;

        if ( state_data->using_gzip )
        {
            state_data->filter_buf.push(boost::iostreams::gzip_decompressor());
        }

        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        if ( transfer_encoding & TE_Chunked )
        {
            // Must prime timeout for async actions.
            auto race_preventer = fsm.SetAsyncTimeout("read response content 1",
                fsm.get_timeout_seconds());

            boost::asio::async_read_until(
                *fsm.get_socket_wrapper(), *state_data->read_buffer, "\r\n",
                [fsmp, state_data, race_preventer, start_time](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred
                    )
                {
                    HandleContentChunkSizeRead(*fsmp, state_data,
                        race_preventer, 0, start_time, Duration::zero(),
                        bytes_transferred, ec );
                });
        }
        else
        {
            uint64_t max_read_size = kMaxUnknownReadLength;
            if ( state_data->using_content_length )
            {
                max_read_size = std::min(
                    state_data->content_length,
                    kMaxUnknownReadLength );
            }

            // Must prime timeout for async actions.
            auto race_preventer = fsm.SetAsyncTimeout("read response content 2",
                fsm.get_timeout_seconds());
            const auto previously_transferred = state_data->read_buffer->size();

            boost::asio::async_read(
                *fsm.get_socket_wrapper(),
                *state_data->read_buffer,
                boost::asio::transfer_exactly(max_read_size),
                [fsmp, state_data, race_preventer, start_time,
                    previously_transferred](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred
                    )
                {
                    HandleContentRead(*fsmp, state_data, race_preventer, 0,
                        start_time, previously_transferred, bytes_transferred,
                        ec);
                });
        }
    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        assert(state_data_);
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    ReadContentDataPointer state_data_;
};


}  // namespace detail
}  // namespace http
}  // namespace mf
