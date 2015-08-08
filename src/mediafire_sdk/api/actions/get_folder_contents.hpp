#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TRequest>
class GetFolderContents : public Coroutine
{
public:
    // enum class to determine if you want to get the files and/or folders of
    // the folder.
    enum class FilesOrFoldersOrBoth
    {
        Files,
        Folders,
        Both
    };

public:
    // Some convenience typedefs
    using RequestType = TRequest;
    using ResponseType = typename RequestType::ResponseType;
    using ResponseDataType = typename RequestType::ResponseDataType;

    using ContentType = typename RequestType::ContentType;

    using File = typename ResponseDataType::File;
    using Folder = typename ResponseDataType::Folder;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType(const std::string & folder_key,
                  FilesOrFoldersOrBoth files_or_folders_or_both,
                  uint32_t chunk_num,
                  const std::error_code & error_code,
                  const boost::optional<std::string> & error_string);

        std::string folder_key;
        FilesOrFoldersOrBoth files_or_folders_or_both;
        uint32_t chunk_num;
        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using CallbackType = std::function<void(
            const std::vector<typename ResponseDataType::File> & files,
            const std::vector<typename ResponseDataType::Folder> & folders,
            const std::vector<ErrorType> & errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetFolderContents> Create(
            SessionMaintainer * stm,
            const std::string & folder_key,
            const FilesOrFoldersOrBoth & files_or_folders_or_both,
            CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetFolderContents(SessionMaintainer * stm,
                      const std::string & folder_key,
                      const FilesOrFoldersOrBoth & files_or_folders_or_both,
                      CallbackType && callback);

    void HandleFolderGetContentsFiles(const ResponseType & response,
                                      uint32_t chunk_number);
    void HandleFolderGetContentsFolders(const ResponseType & response,
                                        uint32_t chunk_number);

    void CoroutineBody(pull_type & yield) override;

private:
    SessionMaintainer * stm_;

    std::string folder_key_;

    std::vector<File> files_;
    std::vector<Folder> folders_;

    std::vector<ErrorType> errors_;

    FilesOrFoldersOrBoth files_or_folders_or_both_;

    CallbackType callback_;
    bool file_chunks_remaining_ = false;
    bool folder_chunks_remaining_ = false;

    SessionMaintainer::Request files_request_ = nullptr;
    SessionMaintainer::Request folders_request_ = nullptr;
};

}  // namespace mf
}  // namespace api

#include "get_folder_contents_impl.hpp"
