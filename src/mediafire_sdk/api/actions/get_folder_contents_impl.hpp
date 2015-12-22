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
GetFolderContents<TRequest>::~GetFolderContents<TRequest>()
{
    std::cout << __FUNCTION__ << ": Destroying: " << this << std::endl;
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
          callback_(callback),
          request_(nullptr)
{
}

template <class TRequest>
void GetFolderContents<TRequest>::Cancel()
{
    // cancelled_ = true;

    // if (files_request_ != nullptr)
    //     files_request_->Cancel();

    // if (folders_request_ != nullptr)
    //     folders_request_->Cancel();
}

// template <class TRequest>
// void GetFolderContents<TRequest>::HandleFolderGetContentsFiles(
//         const ResponseType & response,
//         uint32_t chunk_number)
// {
//     if (!response.response_data)
//     {
//         // If there was an error, insert into vector and
//         // propagate at the callbacks
//         errors_.push_back(ErrorType(folder_key_, FilesOrFoldersOrBoth::Files,
//                                     chunk_number, response.error_code,
//                                     response.error_string));
//     }
//     else
//     {
//         const auto & data = *response.response_data;

//         // Insert the list of files from the response
//         // into our own list
//         files_.insert(std::end(files_), std::begin(data.files),
//                       std::end(data.files));

//         // Set flag if the file chunks are remaining
//         if (data.chunks_remaining
//             == mf::api::folder::get_content::ChunksRemaining::LastChunk)
//             file_chunks_remaining_ = false;
//     }

//     files_request_ = nullptr;  // Must free files_request_ or coroutine
//     cannot
//     // be destructed.

//     // Resume the coroutine
//     Resume();
// }

// template <class TRequest>
// void GetFolderContents<TRequest>::HandleFolderGetContentsFolders(
//         const ResponseType & response,
//         uint32_t chunk_number)
// {
//     if (!response.response_data)
//     {
//         // If there was an error, insert into vector and
//         // propagate at the callback.
//         std::string error_str = "No error string provided";
//         if (response.error_string)
//             error_str = *response.error_string;

//         errors_.push_back(ErrorType(folder_key_,
//         FilesOrFoldersOrBoth::Folders,
//                                     chunk_number, response.error_code,
//                                     error_str));
//     }
//     else
//     {
//         const auto & data = *response.response_data;

//         // Insert the list of folders from the response
//         // into our own list
//         folders_.insert(std::end(folders_), std::begin(data.folders),
//                         std::end(data.folders));

//         // Set flag if folder chunks are remaining
//         if (data.chunks_remaining
//             == mf::api::folder::get_content::ChunksRemaining::LastChunk)
//             folder_chunks_remaining_ = false;
//     }

//     folders_request_ = nullptr;  // Must free folders_request_ or coroutine
//     // cannot be destructed.

//     // Resume the coroutine
//     Resume();
// }

template <class TRequest>
void GetFolderContents<TRequest>::CoroutineBody(pull_type & yield)
{
    // // The chunk number for each content type of call respectively
    // int files_chunk_num = 1;
    // int folders_chunk_num = 1;

    // // Do we still have work left to queue?
    // while ((folder_chunks_remaining_ || file_chunks_remaining_) &&
    // !cancelled_)
    // {
    //     int yield_count = 0;

    //     // Files or Both
    //     if ((files_or_folders_or_both_ == FilesOrFoldersOrBoth::Files
    //          || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
    //         && file_chunks_remaining_)
    //     {
    //         std::function<void(const ResponseType & response)>
    //                 HandleFolderGetContentsFiles =
    //                         [this, self, files_chunk_num](
    //                                 const ResponseType & response)
    //         {
    //             this->HandleFolderGetContentsFiles(response,
    //             files_chunk_num);
    //         };

    //         files_request_
    //                 = stm_->Call(RequestType(folder_key_, files_chunk_num,
    //                                          ContentType::Files),
    //                              HandleFolderGetContentsFiles);

    //         if (cancelled_)
    //             files_request_->Cancel();

    //         ++yield_count;
    //     }

    //     // Folders or Both
    //     if ((files_or_folders_or_both_ == FilesOrFoldersOrBoth::Folders
    //          || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
    //         && folder_chunks_remaining_)
    //     {
    //         std::function<void(const ResponseType & response)>
    //                 HandleFolderGetContentsFolders =
    //                         [this, self, folders_chunk_num](
    //                                 const ResponseType & response)
    //         {
    //             this->HandleFolderGetContentsFolders(response,
    //                                                  folders_chunk_num);
    //         };

    //         folders_request_
    //                 = stm_->Call(RequestType(folder_key_, folders_chunk_num,
    //                                          ContentType::Folders),
    //                              HandleFolderGetContentsFolders);
    //         if (cancelled_)
    //             folders_request_->Cancel();

    //         ++yield_count;
    //     }

    //     // Yield this coroutine once if Files or Folders, twice if
    //     // Both.
    //     for (int i = 0; i < yield_count; ++i)
    //         yield();

    //     // Update the chunk number
    //     if (folder_chunks_remaining_)
    //         ++folders_chunk_num;

    //     // Update the chunk number
    //     if (file_chunks_remaining_)
    //         ++files_chunk_num;
    // }

    // // Coroutine is done, so call the callback.
    // callback_(files_, folders_, errors_);

    switch (files_or_folders_or_both_)
    {
        case FilesOrFoldersOrBoth::Files:
            FilesBody(yield);

            std::cout << __FUNCTION__
                      << ": Returned from files body.  Calling callback."
                      << std::endl;

            // Coroutine is done, so call the callback.
            (*callback_)(files_, folders_, errors_);

            std::cout << __FUNCTION__ << ": Returned from files only callback."
                      << std::endl;
            break;

        case FilesOrFoldersOrBoth::Folders:
            FoldersBody(yield);

            std::cout << __FUNCTION__
                      << ": Returned from folders body.  Calling callback."
                      << std::endl;

            // Coroutine is done, so call the callback.
            (*callback_)(files_, folders_, errors_);

            std::cout << __FUNCTION__
                      << ": Returned from folders only callback." << std::endl;
            break;

        case FilesOrFoldersOrBoth::Both:
            FilesAndFoldersBody(yield);

            std::cout << __FUNCTION__
                      << ": Returned from both body.  Calling callback."
                      << std::endl;

            // Coroutine is done, so call the callback.
            (*callback_)(files_, folders_, errors_);

            std::cout << __FUNCTION__ << ": Returned from both callback."
                      << std::endl;
            break;
    }

    // Free callback for correct memory deallocation
    callback_ = nullptr;
}

template <class TRequest>
void GetFolderContents<TRequest>::FilesBody(pull_type & yield)
{
    int chunk_id = 1;
    bool has_remaining = true;

    while (has_remaining && errors_.empty())
    {
        std::cout << __FUNCTION__ << ": Requesting files." << std::endl;

        stm_->Call(
                RequestType(folder_key_, chunk_id, ContentType::Files),
                [this, chunk_id, &has_remaining](const ResponseType & response)
                {
                    std::cout << __FUNCTION__ << ": Received files."
                              << std::endl;

                    if (!response.response_data)
                    {
                        // If there was an error, insert into vector and
                        // propagate at the callback.
                        std::string error_str = "No error string provided";
                        if (response.error_string)
                            error_str = *response.error_string;

                        errors_.push_back(ErrorType(
                                folder_key_, FilesOrFoldersOrBoth::Files,
                                chunk_id, response.error_code, error_str));
                    }
                    else
                    {
                        const auto & data = *response.response_data;

                        // Insert the list of folders from the response
                        // into our own list
                        files_.insert(std::end(files_), std::begin(data.files),
                                      std::end(data.files));

                        // Set flag if folder chunks are remaining
                        has_remaining = data.chunks_remaining
                                        != mf::api::folder::get_content::
                                                   ChunksRemaining::LastChunk;
                    }

                    // Resume the coroutine
                    Resume();
                });

        std::cout << __FUNCTION__ << ": Yielding" << std::endl;
        yield();
        std::cout << __FUNCTION__ << ": Returned" << std::endl;

        ++chunk_id;
    }
}

template <class TRequest>
void GetFolderContents<TRequest>::FoldersBody(pull_type & yield)
{
    int chunk_id = 1;
    bool has_remaining = true;

    while (has_remaining && errors_.empty())
    {
        std::cout << __FUNCTION__ << ": Requesting folders." << std::endl;

        stm_->Call(
                RequestType(folder_key_, chunk_id, ContentType::Folders),
                [this, chunk_id, &has_remaining](const ResponseType & response)
                {
                    std::cout << __FUNCTION__ << ": Received folders."
                              << std::endl;

                    if (!response.response_data)
                    {
                        // If there was an error, insert into vector and
                        // propagate at the callback.
                        std::string error_str = "No error string provided";
                        if (response.error_string)
                            error_str = *response.error_string;

                        errors_.push_back(ErrorType(
                                folder_key_, FilesOrFoldersOrBoth::Folders,
                                chunk_id, response.error_code, error_str));
                    }
                    else
                    {
                        const auto & data = *response.response_data;

                        // Insert the list of folders from the response
                        // into our own list
                        folders_.insert(std::end(folders_),
                                        std::begin(data.folders),
                                        std::end(data.folders));

                        // Set flag if folder chunks are remaining
                        has_remaining = data.chunks_remaining
                                        != mf::api::folder::get_content::
                                                   ChunksRemaining::LastChunk;
                    }

                    // Resume the coroutine
                    Resume();
                });

        std::cout << __FUNCTION__ << ": Yielding" << std::endl;
        yield();
        std::cout << __FUNCTION__ << ": Returned" << std::endl;
        ++chunk_id;
    }
}

template <class TRequest>
void GetFolderContents<TRequest>::FilesAndFoldersBody(pull_type & yield)
{
    auto self = shared_from_this();

    {
        CallbackType handle_files
                = [this, self](const std::vector<File> & response_files,
                               const std::vector<Folder> &,
                               const std::vector<ErrorType> & errors)
        {
            std::cout << __FUNCTION__ << ": In file co-routine callback."
                      << std::endl;
            files_ = response_files;
            std::copy(std::begin(errors), std::end(errors),
                      std::back_inserter(errors_));

            // Resume the coroutine
            Resume();
        };

        auto get_files = Create(stm_, folder_key_, FilesOrFoldersOrBoth::Files,
                                std::move(handle_files));

        CallbackType handle_folders = [this, self, get_files](
                const std::vector<File> &,
                const std::vector<Folder> & response_folders,
                const std::vector<ErrorType> & errors)
        {
            std::cout << __FUNCTION__ << ": In folder co-routine callback."
                      << std::endl;

            folders_ = response_folders;
            std::copy(std::begin(errors), std::end(errors),
                      std::back_inserter(errors_));

            // Resume the coroutine
            Resume();
        };

        auto get_folders
                = Create(stm_, folder_key_, FilesOrFoldersOrBoth::Folders,
                         std::move(handle_folders));

        get_files->Start();
        get_folders->Start();
    }

    std::cout << __FUNCTION__ << ": Yield 1" << std::endl;
    yield();
    std::cout << __FUNCTION__ << ": Yield 2" << std::endl;
    yield();
    std::cout << __FUNCTION__ << ": Returned from dual yields" << std::endl;
}

}  // namespace mf
}  // namespace api
