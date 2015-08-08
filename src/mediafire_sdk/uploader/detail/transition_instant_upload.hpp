/**
 * @file transition_instant_upload.hpp
 * @author Herbert Jones
 * @brief State machine instant upload transition
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>

#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/upload/instant.hpp"

#include "mediafire_sdk/uploader/upload_request.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"

namespace mf
{
namespace uploader
{
namespace detail
{
namespace upload_transition
{

struct DoInstantUpload
{
    class TargetSetter : public boost::static_visitor<>
    {
    public:
        TargetSetter(mf::api::upload::instant::Request * request)
                : request_(request)
        {
        }

        void operator()(mf::uploader::detail::ParentFolderKey & variant) const
        {
            request_->SetTargetParentFolderkey(variant.key);
        }

        void operator()(mf::uploader::detail::CloudPath & variant) const
        {
            request_->SetTargetPath(variant.path);
        }

    private:
        mf::api::upload::instant::Request * request_;
    };

    template <typename Event,
              typename FSM,
              typename SourceState,
              typename TargetState>
    void operator()(Event const &, FSM & fsm, SourceState &, TargetState &)
    {
        namespace instant = mf::api::upload::instant;

        auto request
                = instant::Request(fsm.filename(), fsm.hash(), fsm.filesize());

        switch (fsm.onDuplicateAction())
        {
            case OnDuplicateAction::Fail:
                // This is the default, same as "skip" and doesn't need to be
                // set.
                break;
            case OnDuplicateAction::Replace:
                request.SetActionOnDuplicate(
                        instant::ActionOnDuplicate::Replace);
                break;
            case OnDuplicateAction::AutoRename:
                request.SetActionOnDuplicate(instant::ActionOnDuplicate::Keep);
                break;
            default:
                assert(!"Invalid duplicate action.");
                break;
        }

        UploadTarget target_folder = fsm.targetFolder();
        boost::apply_visitor(TargetSetter(&request), target_folder);

        auto fsmp = fsm.AsFrontShared();

        fsm.GetSessionMaintainer()->Call(
                request, [fsmp](const instant::Response & response)
                {
                    if (!response.response_data)
                    {
                        fsmp->ProcessEvent(
                                event::Error{response.error_code,
                                             "Failed to instant upload file."});
                    }
                    else
                    {
                        const auto & response_data = *response.response_data;

                        fsmp->ProcessEvent(event::InstantSuccess{
                                response_data.quickkey,
                                response_data.filename,
                                response_data.new_device_revision});
                    }
                });
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
