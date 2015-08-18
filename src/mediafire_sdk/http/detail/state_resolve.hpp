/**
 * @file state_resolve.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>

#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

class ResolveData
{
public:
    ResolveData(
            boost::asio::io_service * io_service
        ) :
        cancelled(false),
        resolver(*io_service)
    {}

    bool cancelled;
    asio::ip::tcp::resolver resolver;
};
using ResolveDataPointer = std::shared_ptr<ResolveData>;

template <typename FSM>
void HandleResolve(
        FSM & fsm,
        ResolveDataPointer state_data,
        RacePreventer race_preventer,
        const boost::system::error_code& err,
        asio::ip::tcp::resolver::iterator endpoint_iterator
    )
{
    using mf::http::http_error;

    bool is_first = race_preventer.IsFirst();

#if defined(OUTPUT_DEBUG) || defined(DEBUG_API_CONNECTION)
    std::cout << "In HandleResolve- Canceled: " << state_data->cancelled
        << ", First: " << is_first << ", err: \"" << err.message() << "\""
        << std::endl;
#endif

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Skip if cancelled due to timeout.
    if ( ! is_first ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsm.ProcessEvent(ResolvedEvent{endpoint_iterator});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while resolving url(" << fsm.get_url() << ").";
        ss << " Error: " << err.message();
        fsm.ProcessEvent(ErrorEvent{
                make_error_code(
                    http_error::UnableToResolve ),
                ss.str()
            });
    }
}

class Resolve : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const&, FSM & fsm)
    {
        auto state_data = std::make_shared<ResolveData>(fsm.get_work_io_service());
        state_data_ = state_data;

        const Url * url = fsm.get_parsed_url();

        assert(url);

        std::string host = url->host();
        std::string port = url->port();
        if ( port.empty() )
            port = url->scheme();

        // Set proxy.
        if ( fsm.UsingProxy() )
        {
            host = fsm.CurrentProxy().host;
            port = boost::lexical_cast<std::string>(fsm.CurrentProxy().port);
        }

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("resolving", kResolvingTimeout);
        auto fsmp = fsm.AsFrontShared();

#if defined(OUTPUT_DEBUG) || defined(DEBUG_API_CONNECTION)
        std::cout << "Attempting name resolution: " << host << std::endl;
#endif

        asio::ip::tcp::resolver::query query(host, port);
        state_data->resolver.async_resolve(
            query,
            [fsmp, state_data, race_preventer](
                    const boost::system::error_code& ec,
                    boost::asio::ip::tcp::resolver::iterator iterator
                )
            {
                HandleResolve( *fsmp, state_data, race_preventer, ec, iterator);
            });
    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    ResolveDataPointer state_data_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
