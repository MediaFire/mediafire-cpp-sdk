/**
 * @file transition_check.hpp
 * @author Herbert Jones
 * @brief State machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>

#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/api/upload/check.hpp"
#include "mediafire_sdk/uploader/error.hpp"
#include "mediafire_sdk/uploader/upload_request.hpp"
#include "mediafire_sdk/uploader/detail/types.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"
#include "mediafire_sdk/uploader/detail/upload_target.hpp"
#include "mediafire_sdk/utils/string.hpp"

namespace mf
{
namespace uploader
{
namespace detail
{
namespace upload_transition
{

template <typename FSM>
void HandleCheck(FSM & fsm, const mf::api::upload::check::Response & response)
{
    assert(!fsm.filename().empty());

    // This function should never receive a bad response.
    assert(response.response_data);

    const auto & response_data = *response.response_data;

    namespace chk = mf::api::upload::check;

    if (response_data.file_exists == chk::FilenameInFolder::Yes
        && response_data.hash_different == chk::FileExistsWithDifferentHash::No
        && response_data.duplicate_quickkey)
    {
        // Filename same and hash same.  We are already done here.
        fsm.ProcessEvent(event::AlreadyUploaded{
                *response_data.duplicate_quickkey, fsm.filename()});
    }
    else if (response_data.storage_limit_exceeded
             == chk::StorageLimitExceeded::Yes)
    {
        // Can't upload, insufficient space.

        fsm.ProcessEvent(event::Error{
                make_error_code(mf::uploader::errc::InsufficientCloudStorage),
                "Account lacks sufficient storage for upload to cloud"});
    }
    else if (response_data.file_exists == chk::FilenameInFolder::Yes
             && fsm.onDuplicateAction() != OnDuplicateAction::Replace
             && fsm.onDuplicateAction() != OnDuplicateAction::AutoRename)
    {
        switch (fsm.onDuplicateAction())
        {
            case OnDuplicateAction::Fail:
                fsm.ProcessEvent(event::Error{
                        make_error_code(mf::uploader::errc::FileExistInFolder),
                        "File already exists in folder."});
                break;
            default:
                assert(!"Unhandled duplicate action");
        }
    }
    else if (response_data.hash_exists == chk::HashAlreadyInSystem::Yes)
    {
        fsm.ProcessEvent(event::InstantUpload{});
    }
    else if (fsm.chunkRanges().size() > 1)
    {
        if (!response_data.resumable)
        {
            assert(!"bitmap missing when it should exist");
            fsm.ProcessEvent(event::NeedsSingleUpload{});
        }
        else
        {
            const auto & resumable = *response_data.resumable;
            if (resumable.number_of_units != fsm.chunkRanges().size())
            {
                assert(!"unit count does not match chunk count");
                fsm.ProcessEvent(event::NeedsSingleUpload{});
            }
            else
            {
                fsm.SetBitmap(resumable.words);

                // Do upload
                fsm.ProcessEvent(event::NeedsChunkUpload{});
            }
        }
    }
    else
    {
        fsm.ProcessEvent(event::NeedsSingleUpload{});
    }
}

struct Check
{
    class TargetSetter : public boost::static_visitor<>
    {
    public:
        TargetSetter(::mf::api::upload::check::Request * request)
                : request_(request)
        {
        }

        void operator()(ParentFolderKey variant) const
        {
            request_->SetTargetParentFolderkey(variant.key);
        }

        void operator()(CloudPath variant) const
        {
            request_->SetPath(mf::utils::path_to_utf8(variant.path));
        }

    private:
        ::mf::api::upload::check::Request * request_;
    };

    template <typename Event,
              typename FSM,
              typename SourceState,
              typename TargetState>
    void operator()(Event const &, FSM & fsm, SourceState &, TargetState &)
    {
        auto request = ::mf::api::upload::check::Request(fsm.filename());
        request.SetHash(fsm.hash());
        request.SetFilesize(fsm.filesize());

        // Set target folderpath
        auto target = fsm.targetFolder();
        boost::apply_visitor(TargetSetter(&request), target);

        if (fsm.chunkRanges().size() > 1)
        {
            // If multiple chunks, we should set resumable
            request.SetResumable(
                    ::mf::api::upload::check::Resumable::Resumable);
        }

        auto fsmp = fsm.AsFrontShared();

        fsm.GetSessionMaintainer()->Call(
                request,
                [fsmp](const mf::api::upload::check::Response & response)
                {
                    if (!response.response_data)
                    {
                        assert(response.error_code);

                        std::string error_str = "unknown error";
                        if (response.error_string)
                            error_str = *response.error_string;

                        fsmp->ProcessEvent(
                                event::Error{response.error_code, error_str});
                    }
                    else
                    {
                        HandleCheck(*fsmp, response);
                    }
                });
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
