/**
 * @file hasher.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "hasher.hpp"

#include <iostream>
#include <memory>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/filesystem.hpp"
#include "boost/msm/back/state_machine.hpp"
#include "boost/msm/front/state_machine_def.hpp"
#include <boost/msm/front/functor_row.hpp>
#include "boost/mpl/vector/vector10.hpp"

#include "mediafire_sdk/utils/error.hpp"
#include "mediafire_sdk/utils/fileio.hpp"
#include "mediafire_sdk/utils/sha256_hasher.hpp"
#include "mediafire_sdk/uploader/detail/stepping.hpp"

#include "mediafire_sdk/uploader/detail/hasher_events.hpp"
#include "mediafire_sdk/uploader/detail/hasher_transitions.hpp"
#include "mediafire_sdk/uploader/error.hpp"

namespace asio  = ::boost::asio;
namespace mpl   = ::boost::mpl;
namespace msm   = ::boost::msm;

namespace {

using msm::front::Row;
using msm::front::none;

namespace ht = mf::uploader::detail::hash_transition;
namespace he = mf::uploader::detail::hash_event;

// front-end: define the FSM structure
class Hasher_ :
    public std::enable_shared_from_this<Hasher_>,
    public msm::front::state_machine_def<Hasher_>
{
public:
    // -- States ---------------------------------------------------------------
    // -- Entry State --
    struct Unstarted : public msm::front::state<> {};

    // -- Normal States --
    struct Started : public msm::front::state<> {};
    struct Reading : public msm::front::state<> {};

    // -- Exit States --
    struct Success : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const & src_evt, FSM& fsm)
        {
            auto & state = src_evt.state;
            mf::uploader::FileHashes hashes;

            hashes.path = state->filepath;
            hashes.file_size = state->filesize;
            hashes.hash = src_evt.sha256_hash;
            hashes.chunk_hashes = state->chunk_hashes;
            hashes.chunk_ranges = state->chunk_ranges;

            fsm.callback(std::error_code(), hashes);
        }
    };

    struct Error : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const & src_evt, FSM& fsm)
        {
            fsm.callback(src_evt.error_code, boost::none);
        }
    };

    // -- END States -----------------------------------------------------------

    // the initial state of the Hasher_ SM. Must be defined
    typedef Unstarted initial_state;

    // Transition table for Hasher_
    struct transition_table : mpl::vector6<
        //    Start     , Event           , Next    , Action        , Guard
        // ------------ , --------------- , ------- , ------------- , -----
        Row < Unstarted , he::StartHash   , Started , ht::OpenFile  , none  >,
        // ------------ , --------------- , ------- , ------------- , -----
        Row < Started   , he::ReadNext    , Reading , ht::ReadFile  , none  >,
        Row < Started   , he::Error       , Error   , none          , none  >,
        // ------------ , --------------- , ------- , ------------- , -----
        Row < Reading   , he::HashSuccess , Success , none          , none  >,
        Row < Reading   , he::ReadNext    , Reading , ht::ReadFile  , none  >,
        Row < Reading   , he::Error       , Error   , none          , none  >
        // ------------ , --------------- , ------- , ------------- , -----
            > {};

    // Replaces the default no-transition response. Use this if you don't want
    // the machine to assert on an unexpected event.
    template <typename FSM, typename Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cerr << "In Hasher: no transition from state " << state
            << " on event " << typeid(e).name() << std::endl;
        assert(!"improper transition in hasher state machine parent");
    }

    mf::uploader::Hasher::Callback callback;
};

// Pick a back-end
using HashMachine = boost::msm::back::state_machine<Hasher_>;

}  // namespace

namespace mf {
namespace uploader {

struct HasherImpl
{
    std::shared_ptr<HashMachine> sm;
    detail::hash_event::HasherStateData state;
};

Hasher::Hasher(
        boost::asio::io_service * io_service,
        boost::filesystem::path filepath,
        uint64_t filesize,
        std::time_t mtime,
        Callback callback
    ) :
    impl_(std::make_shared<HasherImpl>())
{
    auto sm = std::make_shared<HashMachine>();
    sm->callback = callback;

    auto state = std::make_shared<detail::hash_event::HasherStateData_>();
    state->io_service = io_service;
    state->filepath = filepath;
    state->filesize = filesize;
    state->mtime = mtime;

    state->chunk_ranges = mf::uploader::detail::ChunkRanges(filesize);

    impl_->sm = std::move(sm);
    impl_->state = std::move(state);
}

Hasher::~Hasher()
{
}

Hasher::Pointer Hasher::Create(
        boost::asio::io_service * fileread_io_service,
        boost::filesystem::path filepath,
        uint64_t filesize,
        std::time_t mtime,
        Callback callback
    )
{
    return Pointer(new Hasher( fileread_io_service, filepath, filesize, mtime,
            callback));
}

void Hasher::Start()
{
    impl_->sm->process_event(detail::hash_event::StartHash{
        impl_->state
        });
}

void Hasher::Cancel()
{
    impl_->sm->process_event( detail::hash_event::Error{
        impl_->state,
        make_error_code(mf::uploader::errc::Cancelled),
        "Call cancelled"
        });
}

}  // namespace uploader
}  // namespace mf
