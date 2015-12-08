namespace mf
{
namespace api
{
template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
std::shared_ptr<PollChanges<TDeviceGetStatusRequest,
                            TDeviceGetChangesRequest,
                            TFolderGetInfoRequest,
                            TFileGetInfoRequest>>
PollChanges<TDeviceGetStatusRequest,
            TDeviceGetChangesRequest,
            TFolderGetInfoRequest,
            TFileGetInfoRequest>::Create(SessionMaintainer * stm,
                                         uint32_t revision,
                                         std::shared_ptr<WorkManager>
                                                 work_manager,
                                         CallbackType && callback)
{
    return std::shared_ptr<PollChanges>(
            new PollChanges(stm, revision, work_manager, std::move(callback)));
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
PollChanges<TDeviceGetStatusRequest,
            TDeviceGetChangesRequest,
            TFolderGetInfoRequest,
            TFileGetInfoRequest>::PollChanges(SessionMaintainer * stm,
                                              uint32_t revision,
                                              std::shared_ptr<WorkManager>
                                                      work_manager,
                                              CallbackType && callback)
        : stm_(stm),
          revision_(revision),
          work_manager_(work_manager),
          callback_(callback)
{
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollChanges<TDeviceGetStatusRequest,
                 TDeviceGetChangesRequest,
                 TFolderGetInfoRequest,
                 TFileGetInfoRequest>::
        HandleGetChangesDevice(
                uint32_t latest_device_revision,
                const std::vector<File> & updated_files,
                const std::vector<Folder> & updated_folders,
                const std::vector<File> & deleted_files,
                const std::vector<Folder> & deleted_folders,
                const std::vector<DeviceGetStatusErrorType> & get_status_errors,
                const std::vector<DeviceGetChangesErrorType> &
                        get_changes_errors)
{
    latest_device_revision_ = latest_device_revision;

    get_status_errors_ = get_status_errors;
    get_changes_errors_ = get_changes_errors;

    updated_files_ = updated_files;
    deleted_files_ = deleted_files;
    deleted_folders_ = deleted_folders;

    // updated_folders_ = updated_folders;
    // Hack around updated_folders containing trash folder key
    // Also hack around deleted folders showing up in updates
    // -_-
    for (const auto & updated_folder : updated_folders)
    {
        bool is_trash = updated_folder.folderkey == "trash";
        bool is_in_trash = updated_folder.parent_folderkey &&
                *updated_folder.parent_folderkey == "trash";
        if (!is_trash && !is_in_trash)
            updated_folders_.push_back(updated_folder);
        if (is_in_trash)
            deleted_folders_.push_back(updated_folder);
    }

    Resume();
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollChanges<TDeviceGetStatusRequest,
                 TDeviceGetChangesRequest,
                 TFolderGetInfoRequest,
                 TFileGetInfoRequest>::
        HandleGetInfoFile(const GetInfoFileResponseType & response,
                          const std::vector<FileGetInfoErrorType> & errors)
{
    if (!errors.empty())
    {
        get_info_file_errors_.insert(std::end(get_info_file_errors_),
                                     std::begin(errors),
                                     std::end(errors));
    }
    else
    {
        updated_files_info_.push_back(response);
    }

    Resume();
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollChanges<TDeviceGetStatusRequest,
                 TDeviceGetChangesRequest,
                 TFolderGetInfoRequest,
                 TFileGetInfoRequest>::
        HandleGetInfoFolder(const GetInfoFolderResponseType & response,
                            const std::vector<FolderGetInfoErrorType> & errors)
{
    if (!errors.empty())
    {
        get_info_folder_errors_.insert(std::end(get_info_folder_errors_),
                                       std::begin(errors),
                                       std::end(errors));
    }
    else
    {
        updated_folders_info_.push_back(response);
    }

    Resume();
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollChanges<TDeviceGetStatusRequest,
                 TDeviceGetChangesRequest,
                 TFolderGetInfoRequest,
                 TFileGetInfoRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    auto HandleGetChangesDevice = [this, self](
            uint32_t latest_device_revision,
            const std::vector<File> & updated_files,
            const std::vector<Folder> & updated_folders,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<DeviceGetChangesErrorType> & get_changes_errors)
    {
        this->HandleGetChangesDevice(latest_device_revision,
                                     updated_files,
                                     updated_folders,
                                     deleted_files,
                                     deleted_folders,
                                     get_status_errors,
                                     get_changes_errors);
    };

    auto get_changes_device = GetChangesDeviceType::Create(
            stm_, revision_, std::move(HandleGetChangesDevice));

    work_manager_->QueueWork(get_changes_device, &yield);

    if (cancelled_)
        work_manager_->Cancel();

    work_manager_->ExecuteWork();

    // Get info on all the files
    for (const auto & file : updated_files_)
    {
        auto HandleGetInfoFile =
                [this, self](const GetInfoFileResponseType & response,
                             const std::vector<FileGetInfoErrorType> & errors)
        {
            this->HandleGetInfoFile(response, errors);
        };

        auto get_info_file = GetInfoFileType::Create(
                stm_, file.quickkey, std::move(HandleGetInfoFile));
        work_manager_->QueueWork(get_info_file, &yield);
    }

    if (cancelled_)
        work_manager_->Cancel();

    work_manager_->ExecuteWork();

    // Get info on all the folders
    for (const auto & folder : updated_folders_)
    {
        auto HandleGetInfoFolder =
                [this, self](const GetInfoFolderResponseType & response,
                             const std::vector<FolderGetInfoErrorType> & errors)
        {
            this->HandleGetInfoFolder(response, errors);
        };

        auto get_info_folder = GetInfoFolderType::Create(
                stm_, folder.folderkey, std::move(HandleGetInfoFolder));
        work_manager_->QueueWork(get_info_folder, &yield);
    }

    if (cancelled_)
        work_manager_->Cancel();

    work_manager_->ExecuteWork();

    // Coroutine is done, so call the callback.
    callback_(latest_device_revision_,
              deleted_files_,
              deleted_folders_,
              updated_files_info_,
              updated_folders_info_,
              get_status_errors_,
              get_changes_errors_,
              get_info_file_errors_,
              get_info_folder_errors_);
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollChanges<TDeviceGetStatusRequest,
                 TDeviceGetChangesRequest,
                 TFolderGetInfoRequest,
                 TFileGetInfoRequest>::Cancel()
{
    cancelled_ = true;

    work_manager_->Cancel();
}

}  // namespace mf
}  // namespace api