#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
class GetChangesDevice : public Coroutine
{
public:
    // Some convenience typedefss

    // device/get_status
    using DeviceGetStatusRequestType = TDeviceGetStatusRequest;
    using DeviceGetStatusResponseType =
            typename DeviceGetStatusRequestType::ResponseType;

    // device/get_changes
    using DeviceGetChangesRequestType = TDeviceGetChangesRequest;
    using DeviceGetChangesResponseType =
            typename DeviceGetChangesRequestType::ResponseType;

    using DeviceGetChangesResponseDataType =
            typename DeviceGetChangesRequestType::ResponseDataType;

    using File = typename DeviceGetChangesResponseDataType::File;
    using Folder = typename DeviceGetChangesResponseDataType::Folder;

    // The struct for the errors we might return
    struct DeviceGetStatusErrorType
    {
        DeviceGetStatusErrorType(const std::error_code & error_code,
                                 const std::string & error_string);

        std::error_code error_code;
        std::string error_string;
    };

    // The struct for the errors we might returnu
    struct DeviceGetChangesErrorType
    {
        DeviceGetChangesErrorType(uint32_t start_revision,
                                  const std::error_code & error_code,
                                  const std::string & error_string);

        uint32_t start_revision;
        std::error_code error_code;
        std::string error_string;
    };

    using CallbackType = std::function<void(
            uint32_t latest_device_revision,
            const std::vector<File> & updated_files,
            const std::vector<Folder> & updated_folders,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<DeviceGetChangesErrorType> & get_changes_errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetChangesDevice> Create(
            SessionMaintainer * stm,
            uint32_t latest_known_revision,
            CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetChangesDevice(SessionMaintainer * stm,
                     uint32_t latest_known_revision,
                     CallbackType && callback);

    void CoroutineBody(pull_type & yield) override;

private:
    SessionMaintainer * stm_;

    uint32_t latest_known_revision_;
    uint32_t latest_device_revision_;

    CallbackType callback_;

    std::vector<DeviceGetStatusErrorType> get_status_errors_;
    std::vector<DeviceGetChangesErrorType> get_changes_errors_;

    // Vectors to hold callback data
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

    SessionMaintainer::Request request_ = nullptr;
};

}  // namespace mf
}  // namespace api

#include "get_changes_device_impl.hpp"
