namespace mf
{
namespace api
{

template <class TRequest>
GetFolderContents<TRequest>::ErrorType::ErrorType(
        const std::string & folder_key,
        FilesOrFoldersOrBoth files_or_folders_or_both,
        uint32_t chunk_num,
        const std::error_code & error_code,
        const boost::optional<std::string> & error_string)
        : folder_key(folder_key),
          files_or_folders_or_both(files_or_folders_or_both),
          chunk_num(chunk_num),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<GetFolderContents<TRequest>>
GetFolderContents<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        const FilesOrFoldersOrBoth & files_or_folders_or_both,
        CallbackType && callback)
{
    return std::shared_ptr<GetFolderContents>(new GetFolderContents(
            stm, folder_key, files_or_folders_or_both, std::move(callback)));
}

template <class TRequest>
GetFolderContents<TRequest>::GetFolderContents(
        SessionMaintainer * stm,
        const std::string & folder_key,
        const FilesOrFoldersOrBoth & files_or_folders_or_both,
        CallbackType && callback)
        : stm_(stm),
          folder_key_(folder_key),
          files_or_folders_or_both_(files_or_folders_or_both),
          callback_(callback)
{
    if (files_or_folders_or_both_ == FilesOrFoldersOrBoth::Files
        || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
    {
        file_chunks_remaining_ = true;
    }

    if (files_or_folders_or_both_ == FilesOrFoldersOrBoth::Folders
        || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
    {
        folder_chunks_remaining_ = true;
    }
}

template <class TRequest>
void GetFolderContents<TRequest>::Cancel()
{
    cancelled_ = true;
}

template <class TRequest>
void GetFolderContents<TRequest>::HandleFolderGetContentsFiles(
        const ResponseType & response,
        uint32_t chunk_number)
{
    if (!response.response_data)
    {
        // If there was an error, insert into vector and
        // propagate at the callbacks
        errors_.push_back(ErrorType(folder_key_, FilesOrFoldersOrBoth::Files,
                                    chunk_number, response.error_code,
                                    response.error_string));
    }
    else
    {
        const auto & data = *response.response_data;

        // Insert the list of files from the response
        // into our own list
        files_.insert(std::end(files_), std::begin(data.files),
                      std::end(data.files));

        // Set flag if the file chunks are remaining
        if (data.chunks_remaining
            == mf::api::folder::get_content::ChunksRemaining::LastChunk)
            file_chunks_remaining_ = false;
    }

    files_request_ = nullptr;  // Must free files_request_ or coroutine cannot
    // be destructed.

    // Resume the coroutine
    Resume();
}

template <class TRequest>
void GetFolderContents<TRequest>::HandleFolderGetContentsFolders(
        const ResponseType & response,
        uint32_t chunk_number)
{
    if (!response.response_data)
    {
        // If there was an error, insert into vector and
        // propagate at the callback.
        std::string error_str = "No error string provided";
        if (response.error_string)
            error_str = *response.error_string;

        errors_.push_back(ErrorType(folder_key_, FilesOrFoldersOrBoth::Folders,
                                    chunk_number, response.error_code,
                                    error_str));
    }
    else
    {
        const auto & data = *response.response_data;

        // Insert the list of folders from the response
        // into our own list
        folders_.insert(std::end(folders_), std::begin(data.folders),
                        std::end(data.folders));

        // Set flag if folder chunks are remaining
        if (data.chunks_remaining
            == mf::api::folder::get_content::ChunksRemaining::LastChunk)
            folder_chunks_remaining_ = false;
    }

    folders_request_ = nullptr;  // Must free folders_request_ or coroutine
    // cannot be destructed.

    // Resume the coroutine
    Resume();
}

template <class TRequest>
void GetFolderContents<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this(); /* Hold a reference to our object until the
                                       coroutine is complete, otherwise handler
                                       will have invalid reference to this
                                       because the base object has disappeared
                                       from scope */

    // The chunk number for each content type of call respectively
    int files_chunk_num = 1;
    int folders_chunk_num = 1;

    // Do we still have work left to queue?
    while ((folder_chunks_remaining_ || file_chunks_remaining_) && !cancelled_)
    {
        int yield_count = 0;

        // Files or Both
        if ((files_or_folders_or_both_ == FilesOrFoldersOrBoth::Files
             || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
            && file_chunks_remaining_)
        {
            std::function<void(const ResponseType & response)>
                    HandleFolderGetContentsFiles =
                            [this, self, files_chunk_num](
                                    const ResponseType & response)
            {
                this->HandleFolderGetContentsFiles(response, files_chunk_num);
            };

            files_request_
                    = stm_->Call(RequestType(folder_key_, files_chunk_num,
                                             ContentType::Files),
                                 HandleFolderGetContentsFiles);

            ++yield_count;
        }

        // Folders or Both
        if ((files_or_folders_or_both_ == FilesOrFoldersOrBoth::Folders
             || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
            && folder_chunks_remaining_)
        {
            std::function<void(const ResponseType & response)>
                    HandleFolderGetContentsFolders =
                            [this, self, folders_chunk_num](
                                    const ResponseType & response)
            {
                this->HandleFolderGetContentsFolders(response,
                                                     folders_chunk_num);
            };

            folders_request_
                    = stm_->Call(RequestType(folder_key_, folders_chunk_num,
                                             ContentType::Folders),
                                 HandleFolderGetContentsFolders);

            ++yield_count;
        }

        // Yield this coroutine once if Files or Folders, twice if
        // Both.
        for (int i = 0; i < yield_count; ++i)
            yield();

        // Update the chunk number
        if (folder_chunks_remaining_)
            ++folders_chunk_num;

        // Update the chunk number
        if (file_chunks_remaining_)
            ++files_chunk_num;
    }

    if (cancelled_)
    {
#if defined(__MINGW32__)
        std::error_code error_code
                = std::make_error_code(std::errc::interrupted);
#else
        std::error_code error_code
                = std::make_error_code(std::errc::operation_canceled);
#endif
        boost::optional<std::string> error_string(std::string("Cancelled"));
        errors_.push_back(
                ErrorType(folder_key_, files_or_folders_or_both_,
                          std::max(files_chunk_num, folders_chunk_num),
                          error_code, error_string));
    }

    // Coroutine is done, so call the callback.
    callback_(files_, folders_, errors_);
}
}  // namespace mf
}  // namespace api
