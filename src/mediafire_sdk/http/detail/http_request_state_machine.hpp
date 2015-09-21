/**
 * @file http_request_state_machine.hpp
 * @author Herbert Jones
 * @brief Http Request state machine
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

#include "boost/msm/back/state_machine.hpp"
#include "boost/msm/front/state_machine_def.hpp"
#include "boost/msm/front/euml/operator.hpp"
#include "boost/msm/front/functor_row.hpp"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/bind.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/restrict.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/variant/apply_visitor.hpp"

#if ! defined(NDEBUG)
#   include "boost/atomic.hpp"
#endif

#include "mediafire_sdk/http/detail/default_http_headers.hpp"
#include "mediafire_sdk/http/detail/encoding.hpp"
#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/socket_wrapper.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"

#include "mediafire_sdk/http/detail/state_connect.hpp"
#include "mediafire_sdk/http/detail/state_error.hpp"
#include "mediafire_sdk/http/detail/state_initialize.hpp"
#include "mediafire_sdk/http/detail/state_parse_headers.hpp"
#include "mediafire_sdk/http/detail/state_proxy_connect.hpp"
#include "mediafire_sdk/http/detail/state_read_content.hpp"
#include "mediafire_sdk/http/detail/state_read_headers.hpp"
#include "mediafire_sdk/http/detail/state_redirect.hpp"
#include "mediafire_sdk/http/detail/state_resolve.hpp"
#include "mediafire_sdk/http/detail/state_send_header.hpp"
#include "mediafire_sdk/http/detail/state_send_post.hpp"
#include "mediafire_sdk/http/detail/state_ssl_handshake.hpp"

#include "mediafire_sdk/http/detail/transition_config.hpp"

#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/shared_buffer.hpp"

#include "mediafire_sdk/utils/base64.hpp"
#include "mediafire_sdk/utils/string.hpp"

// #define OUTPUT_DEBUG

namespace mf {
namespace http {
namespace detail {


// Forward declarations
class HttpRequestMachine_;

/** Back-end to HttpRequest state machine.  Use this class. */
using HttpRequestMachine =
    boost::msm::back::state_machine<detail::HttpRequestMachine_>;
using StateMachinePointer = std::shared_ptr<HttpRequestMachine>;
using StateMachineWeakPointer = std::weak_ptr<HttpRequestMachine>;


struct HttpRequestMachineConfig
{
    HttpConfig::ConstPointer http_config;
    const std::string & url;
    std::shared_ptr<RequestResponseInterface> response_handler;
    boost::asio::io_service * callback_io_service;
};

namespace hl = mf::http;
namespace msm = boost::msm;
namespace mpl = boost::mpl;
namespace asio = boost::asio;
namespace mfr = boost::msm::front;

using msm::front::Row;
using msm::front::none;
using msm::front::euml::Not_;
using msm::front::euml::And_;

static const uint64_t kMaxUnknownReadLength = 1024 * 8;

float MultiplierFromPercent(float percent)
{
    // If desired bandwidth usage is 95% of total bandwidth:
    // 95 * n = 5
    // n = 5 / 95
    // n = (100 - 95) / 95
    if (percent > 100.0f)
    {
        assert(!"Bandwidth usage percent greater than 100");
        percent = 100.0f;
    }
    else if (percent < 1.0f)
    {
        assert(!"Bandwidth usage percent less than 1");
        percent = 1.0f;
    }
    return ( 100.0f - percent ) / percent;
}

asio::ssl::verify_mode CertAllowToVerifyMode(hl::SelfSigned ss)
{
    switch(ss)
    {
        case hl::SelfSigned::Denied:
            return (asio::ssl::verify_peer |
                asio::ssl::verify_fail_if_no_peer_cert);
        case hl::SelfSigned::Permitted:
            return asio::ssl::verify_none;
        default:
            assert(!"Unknown SelfSigned type");
            return (asio::ssl::verify_peer |
                asio::ssl::verify_fail_if_no_peer_cert);
    }
}

// Guards
struct IsSsl
{
    template <class Fsm,class Evt,class SourceState,class TargetState>
    bool operator()(Evt const& evt, Fsm& fsm, SourceState&,TargetState&)
    {
        return fsm.get_is_ssl();
    }
};
struct ProxyConnectRequired
{
    template <class Fsm,class Evt,class SourceState,class TargetState>
    bool operator()(Evt const&, Fsm& fsm, SourceState&,TargetState&)
    {
        // HTTP proxy CONNECT only needed if authentication is required.
        // HTTPS always needs to go through the CONNECT step.
        if (fsm.UsingProxy())
        {
            const auto & proxy = fsm.CurrentProxy();
            bool needs_authentication = ! proxy.username.empty();

            if ( needs_authentication || fsm.get_is_ssl() )
                return true;
        }
        return false;
    }
};
struct HasPost
{
    template <class Fsm,class Evt,class SourceState,class TargetState>
    bool operator()(Evt const&, Fsm& fsm, SourceState&,TargetState&)
    {
        return static_cast<bool>(fsm.get_post_data())
            || static_cast<bool>(fsm.get_post_interface());
    }
};

// front-end: define the FSM structure
class HttpRequestMachine_ :
    public std::enable_shared_from_this<HttpRequestMachine_>,
    public msm::front::state_machine_def<HttpRequestMachine_>
{
public:
    typedef std::shared_ptr<HttpRequestMachine_> Pointer;

    // Warning: Not possible to add more than 5 arguments here:
    HttpRequestMachine_( const HttpRequestMachineConfig & config ) :
        http_config_(config.http_config),
        work_io_service_(config.http_config->GetWorkIoService()),
        event_strand_(*work_io_service_),
        bw_analyser_(http_config_->GetBandwidthAnalyser()),
        request_creation_time_(sclock::now()),
        transmission_delay_timer_(*work_io_service_),
        transmission_delay_timer_enabled_(false),
        timer_(*work_io_service_),
        timeout_seconds_(60),
        timeout_id_(0),
        callback_(config.response_handler),
        callback_io_service_(config.callback_io_service),
        resolver_(*work_io_service_),
        redirect_policy_(http_config_->GetRedirectPolicy()),
        ssl_ctx_(http_config_->GetSslContext()),
        ssl_verify_mode_(CertAllowToVerifyMode(
                http_config_->SelfSignedCertificatesAllowed())),
        request_method_("GET"),
        url_(config.url),
        original_url_(config.url),
        send_headers_(http_config_->GetDefaultHeaders()),
        http_proxy_(http_config_->GetHttpProxy()),
        https_proxy_(http_config_->GetHttpsProxy()),
        delay_multiplier_(MultiplierFromPercent(
                http_config_->GetBandwidthUsagePercent()))
    {
#if ! defined(NDEBUG)
        const int object_count = request_count_.fetch_add(1,
            boost::memory_order_relaxed);
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "++HttpRequests: " << (object_count+1) << ' ' << url_
            << std::endl;
#       endif
        assert( object_count < 100 );
#endif
    }

    virtual ~HttpRequestMachine_()
    {
#if ! defined(NDEBUG)
        const int object_count = request_count_.fetch_sub(1,
            boost::memory_order_release);
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "--HttpRequests: " << (object_count-1) << ' ' << url_
            << std::endl;
#       endif
        assert(object_count > 0);
#endif
    }

    HttpRequestMachine & AsFront()
    {
        return static_cast<msm::back::state_machine<HttpRequestMachine_>&>(
            *this);
    }

    StateMachinePointer AsFrontShared()
    {
        auto self = shared_from_this();
        return std::static_pointer_cast<HttpRequestMachine>(self);
    }

    StateMachineWeakPointer AsFrontWeak()
    {
        auto self = shared_from_this();
        return StateMachineWeakPointer(
            std::static_pointer_cast<HttpRequestMachine>(self));
    }
    template<typename Event>
    void ProcessEvent(Event evt)
    {
        auto self = shared_from_this();

        event_strand_.dispatch(
            [this, self, evt]()
            {
                msm::back::state_machine<HttpRequestMachine_> &fsm =
                    static_cast<msm::back::state_machine<HttpRequestMachine_>&>(
                        *this);
                fsm.process_event( evt );
            });
    }

    // The list of FSM states

    // Notmal states -----
    struct Unstarted : public msm::front::state<>
    {
#if 0
        // It is possible to add on_entry and on_exit actions.

        template <typename Event, typename FSM>
        void on_entry(Event const& , FSM&)
        {
            std::cout << "entering: Unstarted" << std::endl;
        }

        template <typename Event, typename FSM>
        void on_exit(Event const&, FSM& )
        {
            std::cout << "leaving: Unstarted" << std::endl;
        }
#endif
    };

    // Terminate states -----
    struct FinalError : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const& evt , FSM& machine)  // NOLINT
        {
            assert( machine.event_strand_.running_in_this_thread() );

            // Close connection
            machine.Disconnect();

            if ( machine.transmission_delay_timer_enabled_ )
            {
                machine.transmission_delay_timer_enabled_ = false;
                machine.transmission_delay_timer_.cancel();
            }

            std::shared_ptr<hl::RequestResponseInterface> iface(
                    machine.callback_);
            machine.callback_io_service_->dispatch(
                    [evt, iface](){
                        iface->RequestResponseErrorEvent(
                            evt.code, evt.description );
                    }
                );
            }
    };
    struct Complete : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const& , FSM& machine)  // NOLINT
        {
            assert( machine.event_strand_.running_in_this_thread() );

            // Close connection
            machine.Disconnect();

            std::shared_ptr<hl::RequestResponseInterface> iface(
                    machine.callback_);

            machine.callback_io_service_->dispatch(
                    [iface](){
                        iface->RequestResponseCompleteEvent();
                    }
                );
        }
    };

    // the initial state of the HttpRequestMachine SM. Must be defined
    typedef Unstarted initial_state;

    // Asio Handlers
    void HandleAsyncTimeout(
            RacePreventer race_preventer,
            uint32_t timeout_id,
            const boost::system::error_code& err
        )
    {
        // Skip if other function beat us to it.
        if ( ! race_preventer.IsFirst() ) return;

        // If timeout id doesn't match, then both timeout handler and async
        // handler ended up on the stack at the same time.
        if (!err && timeout_id_ == timeout_id )
        {
            std::ostringstream ss;
            ss << "I/O timeout: " << timeout_reason_;
            ProcessEvent(ErrorEvent{
                    make_error_code( hl::http_error::IoTimeout ),
                    ss.str()
                });
        }
    }

    // Helper actions
    void SetTransactionDelayTimer(
            TimePoint last_write_start,
            TimePoint last_write_end
        )
    {
        const auto length = AsDuration(last_write_end - last_write_start);
        SetTransactionDelayTimer( last_write_end, length );
    }
    void SetTransactionDelayTimer(
        TimePoint last_write_end,
        Duration length
        )
    {
        TimePoint expire_time = last_write_end;

        using std::chrono::duration_cast;
        using std::chrono::microseconds;

        uint32_t mss = duration_cast<microseconds>(length).count();

        // std::cout << "Total microseconds: " << mss << std::endl;

        // Get microseconds to delay
        mss *= delay_multiplier_;

        // std::cout << "Total delay microseconds: " << mss
        //     << std::endl;

        expire_time += microseconds(mss);

        transmission_delay_timer_enabled_ = true;
        transmission_delay_timer_.expires_at( expire_time );
    }
    void SetTransactionDelayTimerWrapper(
            std::function<void()> bind_function,
            const boost::system::error_code& err
        )
    {
        // Since we can't set a timer to a strand, and we must check the
        // enabled flag in the strand for thread safety, we use the strand
        // here.
        if ( ! err )
        {
            event_strand_.dispatch(
                bind_function );
        }
    }
    void ClearAsyncTimeout()
    {
        // Change the id for when handler and timeout end up on stack
        // together.
        ++timeout_id_;

        timer_.cancel();
    }
    RacePreventer SetAsyncTimeout(std::string reason, uint32_t timeout_seconds)
    {
        RacePreventer race_preventer(socket_wrapper_.get());

        // Change the id for when handler and timeout end up on stack
        // together.
        ++timeout_id_;

        timer_.cancel();

        // Keep reason for timeout for debug message.
        timeout_reason_ = reason;

        timer_.expires_from_now(std::chrono::seconds(timeout_seconds));
        timer_.async_wait(
            event_strand_.wrap(
                boost::bind(
                    &HttpRequestMachine_::HandleAsyncTimeout,
                    shared_from_this(),
                    race_preventer,
                    timeout_id_,
                    asio::placeholders::error
                )));
        return race_preventer;
    }
    void Disconnect()
    {
        // Close connection
        if (socket_wrapper_)
        {
            socket_wrapper_->Cancel();
            socket_wrapper_.reset();
        }
    }
    void SetHeader(const std::string & name, const std::string & value)
    {
        using CI = hl::HttpRequest::HeaderContainer::value_type;
        auto it = std::find_if(
                send_headers_.begin(),
                send_headers_.end(),
                [&name](const CI & pair)
                {
                    return boost::iequals(pair.first, name);
                }
            );
        if ( it != send_headers_.end() )
        {
            it->second = value;
        }
        else
        {
            send_headers_.emplace_back(name, value);
        }
    }

    typedef HttpRequestMachine_ m;  // makes transition table cleaner

    // Transition table for HttpRequestMachine
    struct transition_table : mpl::vector<
        //    Start           Event                   Next            Action                Guard                                  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < Unstarted     , ConfigEvent           , Unstarted     , ConfigEventAction   , none                               >,  // NOLINT
        Row < Unstarted     , StartEvent            , Initializing  , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < Initializing  , InitializedEvent      , Resolve       , none                , none                               >,  // NOLINT
        Row < Initializing  , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < Resolve       , ResolvedEvent         , Connect       , none                , none                               >,  // NOLINT
        Row < Resolve       , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < Connect       , ConnectedEvent        , ProxyConnect  , none                , ProxyConnectRequired               >,  // NOLINT
        Row < Connect       , ConnectedEvent        , SendHeader    , none                , And_< Not_<ProxyConnectRequired>,      // NOLINT
                                                                                                  Not_<IsSsl>                 >>,  // NOLINT
        Row < Connect       , ConnectedEvent        , SSLHandshake  , none                , And_< Not_<ProxyConnectRequired>,      // NOLINT
                                                                                                  IsSsl                       >>,  // NOLINT
        Row < Connect       , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < ProxyConnect  , ConnectedEvent        , SendHeader    , none                , Not_<IsSsl>                        >,  // NOLINT
        Row < ProxyConnect  , ConnectedEvent        , SSLHandshake  , none                , IsSsl                              >,  // NOLINT
        Row < ProxyConnect  , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < SSLHandshake  , HandshakeEvent        , SendHeader    , none                , none                               >,  // NOLINT
        Row < SSLHandshake  , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < SendHeader    , HeadersWrittenEvent   , SendPost      , none                , HasPost                            >,  // NOLINT
        Row < SendHeader    , HeadersWrittenEvent   , ReadHeaders   , none                , Not_<HasPost>                      >,  // NOLINT
        Row < SendHeader    , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < SendPost      , PostSent              , ReadHeaders   , none                , none                               >,  // NOLINT
        Row < SendPost      , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < ReadHeaders   , HeadersReadEvent      , ParseHeaders  , none                , none                               >,  // NOLINT
        Row < ReadHeaders   , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < ParseHeaders  , HeadersParsedEvent    , ReadContent   , none                , none                               >,  // NOLINT
        Row < ParseHeaders  , RedirectEvent         , Redirect      , none                , none                               >,  // NOLINT
        Row < ParseHeaders  , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < Redirect      , RedirectedEvent       , Initializing  , none                , none                               >,  // NOLINT
        Row < Redirect      , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < ReadContent   , ContentReadEvent      , Complete      , none                , none                               >,  // NOLINT
        Row < ReadContent   , ErrorEvent            , Error         , none                , none                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
        Row < Error         , RestartEvent          , Initializing  , none                , none                               >,  // NOLINT
        Row < Error         , ErrorEvent            , FinalError    , none                , none                               >   // NOLINT
        //  +---------------+-----------------------+---------------+---------------------+------------------------------------+   // NOLINT
    > {};

    // Replaces the default no-transition response. Use this if you don't
    // want the machine to assert on an unexpected event.
    template <typename FSM, typename Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cerr << "no transition from state " << state
            << " on event " << typeid(e).name() << std::endl;
        assert(!"improper transition in http state machine");
    }

    // Throw exception if machine being used incorrectly.
    template <typename FSM>
    void no_transition(ConfigEvent const& e, FSM&, int state)
    {
        throw std::logic_error("Unable to configure in progress HttpRequest.");
    }

    // -- Accessors ------------------------------------------------------------

    hl::RedirectPolicy get_redirect_policy() const {return redirect_policy_;}
    void set_redirect_policy(hl::RedirectPolicy v) {redirect_policy_=v;}

    std::string get_request_method() const {return request_method_;}
    void set_request_method(std::string v) {request_method_=v;}

    std::shared_ptr<hl::PostDataPipeInterface> get_post_interface() const {return post_interface_;}
    void SetPostInterface(
            std::shared_ptr<mf::http::PostDataPipeInterface> pipe_interface
        )
    {
        post_data_.reset();
        post_interface_ = pipe_interface;
        SetHeader("Content-Length",
            mf::utils::to_string(post_interface_->PostDataSize()));

        request_method_ = "POST";
    }

    hl::SharedBuffer::Pointer get_post_data() const {return post_data_;}
    void SetPostData(
            hl::SharedBuffer::Pointer post_data
        )
    {
        post_data_ = post_data;
        post_interface_.reset();
        SetHeader("Content-Length", mf::utils::to_string(post_data_->Size()));

        request_method_ = "POST";
    }

    uint32_t get_timeout_seconds() const {return timeout_seconds_;}
    void set_timeout_seconds(uint32_t v) {timeout_seconds_=v;}

    const hl::Url * get_parsed_url() const {return parsed_url_.get();}
    void set_parsed_url(std::unique_ptr<hl::Url> url)
    {
        parsed_url_ = std::move(url);
    }

    bool UsingProxy() const
    {
        return (( is_ssl_ && https_proxy_ )
            || ( ! is_ssl_ && http_proxy_ ));
    }

    // Must never call this without verifying proxy exists via UsingProxy first.
    const hl::Proxy & CurrentProxy() const
    {
        assert((is_ssl_ && https_proxy_) || (!is_ssl_ && http_proxy_));

        if ( is_ssl_ )
            return *https_proxy_;
        else
            return *http_proxy_;
    }

    bool get_is_ssl() const {return is_ssl_;}
    void set_is_ssl(bool v) {is_ssl_=v;}

    const std::string & get_url() const {return url_;}
    void set_url(std::string v) {url_=v;}

    asio::io_service::strand * get_event_strand() {return &event_strand_;}

    const TimePoint get_request_creation_time() const
    {return request_creation_time_;}

    std::shared_ptr<SocketWrapper> get_socket_wrapper() const {return socket_wrapper_;}
    void set_socket_wrapper(std::shared_ptr<SocketWrapper> v) {socket_wrapper_=v;}

    const hl::HttpRequest::HeaderContainer & get_headers() const {return send_headers_;}

    hl::BandwidthAnalyserInterface::Pointer get_bw_analyser() const {return bw_analyser_;}

    const asio::ssl::verify_mode get_ssl_verify_mode() const {return ssl_verify_mode_;}

    boost::asio::steady_timer * get_transmission_delay_timer() {return &transmission_delay_timer_;}

    bool get_transmission_delay_timer_enabled() const {return transmission_delay_timer_enabled_;}
    void set_transmission_delay_timer_enabled(bool v) {transmission_delay_timer_enabled_=v;}

    asio::io_service * get_callback_io_service() {return callback_io_service_;}
    std::shared_ptr<hl::RequestResponseInterface> get_callback() const {return callback_;}

    asio::io_service * get_work_io_service() const {return work_io_service_;}

    boost::asio::ssl::context * get_ssl_ctx() const {return ssl_ctx_;}

protected:
#if ! defined(NDEBUG)
    // Counter to keep track of number of HttpRequests
    static boost::atomic<int> request_count_;
#endif

    HttpConfig::ConstPointer http_config_;

    // IOService for work.
    asio::io_service * work_io_service_;

    // Strand for events.
    asio::io_service::strand event_strand_;

    // Track bandwidth
    hl::BandwidthAnalyserInterface::Pointer bw_analyser_;

    const TimePoint request_creation_time_;

    // Use these to delay transmission of data for QOS.
    boost::asio::steady_timer transmission_delay_timer_;
    bool transmission_delay_timer_enabled_;

    // Timeout timer
    boost::asio::steady_timer timer_;
    uint32_t timeout_seconds_;
    uint32_t timeout_id_;
    std::string timeout_reason_;

    // Call to request user action.
    std::shared_ptr<hl::RequestResponseInterface> callback_;

    // IOService for callbacks.
    asio::io_service * callback_io_service_;

    // Asio internals
    asio::ip::tcp::resolver resolver_;

    hl::RedirectPolicy redirect_policy_;

    bool is_ssl_;
    boost::asio::ssl::context * ssl_ctx_;

    std::shared_ptr<SocketWrapper> socket_wrapper_;

    const asio::ssl::verify_mode ssl_verify_mode_;

    // Send data.
    std::string request_method_;
    std::string url_;
    const std::string original_url_;
    std::unique_ptr<hl::Url> parsed_url_;

    hl::SharedBuffer::Pointer post_data_;
    std::shared_ptr<hl::PostDataPipeInterface> post_interface_;

    hl::HttpRequest::HeaderContainer send_headers_;

    boost::optional<hl::Proxy> http_proxy_;
    boost::optional<hl::Proxy> https_proxy_;

    const float delay_multiplier_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
