/**
 * @file transition_config.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include "boost/variant/static_visitor.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"

#include "mediafire_sdk/utils/base64.hpp"
#include "mediafire_sdk/utils/string.hpp"

namespace mf
{
namespace http
{
namespace detail
{

template <typename FSM>
class ConfigEventHandler : public boost::static_visitor<>
{
public:
    explicit ConfigEventHandler(FSM & fsm) : fsm_(fsm) {}

    void operator()(const ConfigEvent::ConfigRedirectPolicy & cfg) const
    {
        fsm_.set_redirect_policy(cfg.redirect_policy);
    }

    void operator()(const ConfigEvent::ConfigRequestMethod & cfg) const
    {
        fsm_.set_request_method(cfg.method);
    }

    void operator()(const ConfigEvent::ConfigHeader & cfg) const
    {
        fsm_.SetHeader(cfg.name, cfg.value);
    }

    void operator()(const ConfigEvent::ConfigPostDataPipe & cfg) const
    {
        fsm_.SetPostInterface(cfg.pdi);
    }

    void operator()(const ConfigEvent::ConfigPostData & cfg) const
    {
        fsm_.SetPostData(cfg.raw_data);
    }

    void operator()( const ConfigEvent::ConfigTimeout & cfg) const
    {
        fsm_.set_timeout_seconds(cfg.timeout_seconds);
    }

    void operator()(const ConfigEvent::StopAfterReadingResponseHeaders &) const
    {
        fsm_.SetStopAfterReadingResponseHeaders();
    }

private:
    FSM & fsm_;
};

struct ConfigEventAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        boost::apply_visitor( ConfigEventHandler<FSM>(fsm), evt.variant );
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
