#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/device/get_foreign_changes.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetForeignChangesRequest>
class GetForeignChangesDevice : public Coroutine
{
public:
    // Some convenience typedefss

    // device/get_changes
    using DeviceGetForeignChangesRequestType = TDeviceGetForeignChangesRequest;
    using DeviceGetForeignChangesResponseType =
            typename DeviceGetForeignChangesRequestType::ResponseType;
    using DeviceGetForeignChangesResponseDataType =
            typename DeviceGetForeignChangesRequestType::ResponseDataType;

    using File = typename DeviceGetForeignChangesResponseDataType::File;
    using Folder = typename DeviceGetForeignChangesResponseDataType::Folder;

    // The struct for the errors we might returnu
    struct DeviceGetForeignChangesErrorType
    {
        DeviceGetForeignChangesErrorType(
                const std::error_code & error_code,
                const boost::optional<std::string> & error_string);

        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using CallbackType = std::function<void(
            uint32_t latest_changes_revision,
            const std::vector<File> & updated_files,
            const std::vector<Folder> & updated_folders,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<DeviceGetForeignChangesErrorType> &
                    get_changes_errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetForeignChangesDevice> Create(
            SessionMaintainer * stm,
            const std::string & contact_key,
            uint32_t latest_known_revision,
            CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetForeignChangesDevice(SessionMaintainer * stm,
                            const std::string & contact_key,
                            uint32_t latest_known_revision,
                            CallbackType && callback);

    void CoroutineBody(pull_type & yield) override;

private:
    SessionMaintainer * stm_;

    std::string contact_key_;

    uint32_t latest_known_revision_;

    CallbackType callback_;

    uint32_t latest_device_revision_ = std::numeric_limits<uint32_t>::max();

    std::vector<DeviceGetForeignChangesErrorType> get_foreign_changes_errors_;

    // Vectors to hold callback data
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

    bool cancelled_ = false;

    SessionMaintainer::Request request_ = nullptr;
};

}  // namespace mf
}  // namespace api

#include "get_foreign_changes_device_impl.hpp"
