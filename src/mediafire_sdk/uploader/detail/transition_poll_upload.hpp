/**
 * @file transition_poll_upload.hpp
 * @author Herbert Jones
 * @brief Transition for poll upload state
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cassert>
#include <string>
#include <system_error>

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/upload/poll_upload.hpp"

#include "mediafire_sdk/uploader/error.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"

namespace
{
const int poll_upload_retry_timeout_seconds = 1;
}  // namespace

namespace mf
{
namespace uploader
{
namespace detail
{
namespace upload_transition
{

// Forward declarations
template <typename FSM>
void StartPoll(const std::string & upload_key, FSM & fsm);

template <typename FSM>
void HandlePollCompleteResponse(
        FSM & fsm,
        const mf::api::upload::poll_upload::Response & response)
{
    assert(response.response_data);
    const auto & response_data = *response.response_data;

    if (!response_data.quickkey)
    {
        fsm.ProcessEvent(
                event::Error{make_error_code(api::api_code::ContentInvalidData),
                             "Successful response missing quickkey"});
    }
    else if (!response_data.revision)
    {
        fsm.ProcessEvent(
                event::Error{make_error_code(api::api_code::ContentInvalidData),
                             "Successful response missing revision"});
    }
    else
    {
        if (response_data.filename)
        {
            // Filename was changed
            fsm.ProcessEvent(event::PollComplete{*response_data.quickkey,
                                                 *response_data.filename,
                                                 *response_data.revision});
        }
        else
        {
            fsm.ProcessEvent(event::PollComplete{*response_data.quickkey,
                                                 fsm.filename(),
                                                 *response_data.revision});
        }
    }
}

template <typename FSM>
void RetryPoll(const std::string & upload_key,
               std::shared_ptr<FSM> fsmp,
               const boost::system::error_code & err)
{
    if (!err)
    {
        StartPoll(upload_key, *fsmp);
    }
}

template <typename FSM>
void HandlePollResponse(const std::string & upload_key,
                        FSM & fsm,
                        const mf::api::upload::poll_upload::Response & response)
{
    if (!response.response_data)
    {
        auto timer = fsm.Timer();

        timer->expires_from_now(
                std::chrono::seconds(poll_upload_retry_timeout_seconds));

        timer->async_wait(boost::bind(&RetryPoll<FSM>, upload_key,
                                      fsm.AsFrontShared(),
                                      boost::asio::placeholders::error));
    }
    else
    {
        const auto & response_data = *response.response_data;
        // if result is negative, it indicates a failure
        if (response_data.result < 0)
        {
            fsm.ProcessEvent(
                    event::Error{std::error_code(response_data.result,
                                                 poll_result_category()),
                                 "Poll upload bad response"});
        }
        else if (response_data.fileerror != 0)
        {
            fsm.ProcessEvent(event::Error{
                    std::error_code(response_data.fileerror,
                                    poll_upload_file_error_category()),
                    "Poll upload file error received"});
        }
        else if (response_data.quickkey)
        {
            HandlePollCompleteResponse(fsm, response);
        }
        else
        {
            auto timer = fsm.Timer();

            timer->expires_from_now(
                    std::chrono::seconds(poll_upload_retry_timeout_seconds));

            timer->async_wait(boost::bind(&RetryPoll<FSM>, upload_key,
                                          fsm.AsFrontShared(),
                                          boost::asio::placeholders::error));
        }
    }
}

template <typename FSM>
void StartPoll(const std::string & upload_key, FSM & fsm)
{
    if (upload_key.empty())
    {
        assert(!"Reached poll upload without upload key");
        fsm.ProcessEvent(
                event::Error{make_error_code(uploader::errc::LogicError),
                             "Filsize unavailable."});
        return;
    }

    auto fsmp = fsm.AsFrontShared();

    fsm.GetSessionMaintainer()->Call(
            mf::api::upload::poll_upload::Request(upload_key),
            [fsmp, upload_key](
                    const mf::api::upload::poll_upload::Response & response)
            {
                HandlePollResponse(upload_key, *fsmp, response);
            });
}

struct PollUpload
{
    template <typename Event,
              typename FSM,
              typename SourceState,
              typename TargetState>
    void operator()(Event const & evt, FSM & fsm, SourceState &, TargetState &)
    {
        StartPoll(evt.upload_key, fsm);
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
