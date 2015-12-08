namespace mf
{
namespace api
{

template <typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::
        DeviceGetForeignChangesErrorType::DeviceGetForeignChangesErrorType(
                const std::error_code & error_code,
                const boost::optional<std::string> & error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetForeignChangesRequest>
std::shared_ptr<GetForeignChangesDevice<TDeviceGetForeignChangesRequest>>
GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::Create(
        SessionMaintainer * stm,
        const std::string & contact_key,
        uint32_t latest_known_revision,
        CallbackType && callback)
{
    return std::shared_ptr<GetForeignChangesDevice>(new GetForeignChangesDevice(
            stm, contact_key, latest_known_revision, std::move(callback)));
}

template <typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::
        GetForeignChangesDevice(SessionMaintainer * stm,
                                const std::string & contact_key,
                                uint32_t latest_known_revision,
                                CallbackType && callback)
        : stm_(stm),
          contact_key_(contact_key),
          latest_known_revision_(latest_known_revision),
          callback_(callback)
{
}

template <typename TDeviceGetForeignChangesRequest>
void GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::Cancel()
{
    cancelled_ = true;

    if (request_ != nullptr)
        request_->Cancel();
}

template <typename TDeviceGetForeignChangesRequest>
void GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::CoroutineBody(
        pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    // device/get_foreign_changes
    uint32_t start_revision = latest_known_revision_;
    uint32_t min_revision = latest_known_revision_ + 1;
    uint32_t max_revision = ((min_revision / 500) + 1) * 500;

    while (start_revision < latest_device_revision_ && !cancelled_)
    {
        std::function<void(const DeviceGetForeignChangesResponseType &
                                   response)> HandleDeviceGetForeignChanges =
                [this, self, start_revision](
                        const DeviceGetForeignChangesResponseType & response)
        {
            if (!response.response_data)
            {
                // If there was an error, insert
                // into vector and
                // propagate at the callback.

                get_foreign_changes_errors_.push_back(
                        DeviceGetForeignChangesErrorType(
                                response.error_code, response.error_string));
            }
            else
            {
                const auto & data = *response.response_data;

                updated_files_.insert(std::end(updated_files_),
                                      std::begin(data.updated_files),
                                      std::end(data.updated_files));

                //                            updated_folders_.insert(
                //                                    std::end(updated_folders_),
                //                                    std::begin(data.updated_folders),
                //                                    std::end(data.updated_folders));

                // HACK around the API including "trash" inside
                // updated_folders
                for (const auto & folder : data.updated_folders)
                {
                    bool is_trash = folder.folderkey == "trash";
                    bool is_in_trash = folder.parent_folderkey &&
                        *folder.parent_folderkey == "trash";
                    if (!is_trash && !is_in_trash)
                        updated_folders_.push_back(folder);
                    if (is_in_trash)
                        deleted_folders_.push_back(folder);
                }

                deleted_files_.insert(std::end(deleted_files_),
                                      std::begin(data.deleted_files),
                                      std::end(data.deleted_files));
                deleted_folders_.insert(std::end(deleted_folders_),
                                        std::begin(data.deleted_folders),
                                        std::end(data.deleted_folders));

                latest_device_revision_ = data.device_revision;
            }

            request_ = nullptr;  // Must free request_ or coroutine cannot be
            // destructed.

            // Resume the coroutine
            Resume();
        };

        request_ = stm_->Call(DeviceGetForeignChangesRequestType(start_revision,
                                                                 contact_key_),
                              HandleDeviceGetForeignChanges);

        if (cancelled_)
            request_->Cancel();

        yield();

        start_revision = max_revision;
        min_revision = start_revision + 1;
        max_revision = ((min_revision / 500) + 1) * 500;
    }

    // Coroutine is done, so call the callback.
    callback_(start_revision, updated_files_, updated_folders_, deleted_files_,
              deleted_folders_, get_foreign_changes_errors_);
}

}  // namespace mf
}  // namespace api
