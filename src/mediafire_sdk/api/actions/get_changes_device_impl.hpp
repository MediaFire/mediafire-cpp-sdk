namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
        DeviceGetStatusErrorType::DeviceGetStatusErrorType(
                const std::error_code & error_code,
                const std::string & error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
        DeviceGetChangesErrorType::DeviceGetChangesErrorType(
                uint32_t start_revision,
                const std::error_code & error_code,
                const std::string & error_string)
        : start_revision(start_revision),
          error_code(error_code),
          error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
std::shared_ptr<GetChangesDevice<TDeviceGetStatusRequest,
                                 TDeviceGetChangesRequest>>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::Create(
        SessionMaintainer * stm,
        uint32_t latest_known_revision,
        CallbackType && callback)
{
    return std::shared_ptr<GetChangesDevice>(new GetChangesDevice(
            stm, latest_known_revision, std::move(callback)));
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
        GetChangesDevice(SessionMaintainer * stm,
                         uint32_t latest_known_revision,
                         CallbackType && callback)
        : stm_(stm),
          latest_known_revision_(latest_known_revision),
          callback_(callback)
{
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
void GetChangesDevice<TDeviceGetStatusRequest,
                      TDeviceGetChangesRequest>::CoroutineBody(pull_type &
                                                                       yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const DeviceGetStatusResponseType & response)>
            HandleDeviceGetStatus =
                    [this, self](const DeviceGetStatusResponseType & response)
    {
        if (!response.response_data)
        {
            // If there was an error, insert into vector and
            // propagate at the callback.
            std::string error_str = "No error string provided";
            if (response.error_string)
                error_str = *response.error_string;

            get_status_errors_.push_back(
                    DeviceGetStatusErrorType(response.error_code, error_str));
        }
        else
        {
            latest_device_revision_ = response.response_data->device_revision;
        }

        request_ = nullptr;  // Must free request_ or coroutine cannot be
        // destructed.

        Resume();
    };

    request_ = stm_->Call(DeviceGetStatusRequestType(), HandleDeviceGetStatus);

    if (cancelled_)
        request_->Cancel();

    yield();

    uint32_t start_revision = latest_known_revision_;
    uint32_t min_revision = latest_known_revision_ + 1;
    uint32_t max_revision = ((min_revision / 500) + 1) * 500;

    while (start_revision < latest_device_revision_ && !cancelled_)
    {
        std::function<void(const DeviceGetChangesResponseType & response)>
                HandleDeviceGetChanges = [this, self, start_revision](
                        const DeviceGetChangesResponseType & response)
        {
            if (!response.response_data)
            {
                // If there was an error, insert
                // into vector and
                // propagate at the callback.
                std::string error_str = "No error string provided ";
                if (response.error_string)
                    error_str = *response.error_string;

                get_changes_errors_.push_back(DeviceGetChangesErrorType(
                        start_revision, response.error_code, error_str));
            }
            else
            {
                const auto & data = *response.response_data;

                updated_files_.insert(std::end(updated_files_),
                                      std::begin(data.updated_files),
                                      std::end(data.updated_files));
                updated_folders_.insert(std::end(updated_folders_),
                                        std::begin(data.updated_folders),
                                        std::end(data.updated_folders));
                deleted_files_.insert(std::end(deleted_files_),
                                      std::begin(data.deleted_files),
                                      std::end(data.deleted_files));
                deleted_folders_.insert(std::end(deleted_folders_),
                                        std::begin(data.deleted_folders),
                                        std::end(data.deleted_folders));
            }

            request_ = nullptr;  // Must free request_ or coroutine cannot be
            // destructed.

            Resume();
        };

        request_ = stm_->Call(DeviceGetChangesRequestType(start_revision),
                              HandleDeviceGetChanges);

        if (cancelled_)
            request_->Cancel();

        yield();

        start_revision = max_revision;
        min_revision = start_revision + 1;
        max_revision = ((min_revision / 500) + 1) * 500;
    }

    // Coroutine is done, so call the callback.
    callback_(latest_device_revision_, updated_files_, updated_folders_,
              deleted_files_, deleted_folders_, get_status_errors_,
              get_changes_errors_);
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
void GetChangesDevice<TDeviceGetStatusRequest,
                      TDeviceGetChangesRequest>::Cancel()
{
    cancelled_ = true;

    if (request_ != nullptr)
        request_->Cancel();
}
}  // namespace mf
}  // namespace api
