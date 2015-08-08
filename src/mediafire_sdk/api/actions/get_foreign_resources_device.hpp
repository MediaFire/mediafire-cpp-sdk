#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetForeignResourcesRequest>
class GetForeignResourcesDevice : public Coroutine
{
public:
    // Some convenience typedefs

    // device/get_foreign_resources
    using DeviceGetForeignResourcesRequestType
            = TDeviceGetForeignResourcesRequest;
    using DeviceGetForeignResourcesResponseType =
            typename DeviceGetForeignResourcesRequestType::ResponseType;
    using DeviceGetForeignResourcesResponseDataType =
            typename DeviceGetForeignResourcesRequestType::ResponseDataType;

    using File = typename DeviceGetForeignResourcesResponseDataType::File;
    using Folder = typename DeviceGetForeignResourcesResponseDataType::Folder;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType(const std::error_code & error_code,
                  const boost::optional<std::string> & error_string);

        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using CallbackType
            = std::function<void(const std::vector<File> & files,
                                 const std::vector<Folder> & folders,
                                 const std::vector<ErrorType> & errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetForeignResourcesDevice> Create(
            SessionMaintainer * stm,
            CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetForeignResourcesDevice(SessionMaintainer * stm,
                              CallbackType && callback);

    void CoroutineBody(pull_type & yield) override;

private:
    SessionMaintainer * stm_;

    CallbackType callback_;

    std::vector<ErrorType> errors_;

    // Vectors to hold callback data
    std::vector<File> files_;
    std::vector<Folder> folders_;

    bool cancelled_ = false;

    SessionMaintainer::Request request_ = nullptr;
};

}  // namespace mf
}  // namespace api

#include "get_foreign_resources_device_impl.hpp"
