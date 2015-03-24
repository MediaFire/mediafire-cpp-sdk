#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TRequest>
class MoveFolder : public Coroutine
{
public:
    // Some convenience typedefs
    using RequestType = TRequest;
    using ResponseType = typename RequestType::ResponseType;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType() {}

        ErrorType(const std::error_code & error_code,
                  boost::optional<std::string> error_string);

        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using CallbackType
            = std::function<void(const ResponseType & response,
                                 const std::vector<ErrorType> & errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<MoveFolder> Create(
            SessionMaintainer * stm,
            const std::string & folder_key,
            const std::string & destination_parent_folder_key,
            CallbackType && callback);

    /**
     *  @brief Starts/resumes the coroutine.
     */
    void operator()() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    MoveFolder(SessionMaintainer * stm,
               const std::string & folder_key,
               const std::string & destination_parent_folder_key,
               CallbackType && callback);

private:
    SessionMaintainer * stm_;

    std::string folder_key_;

    std::string destination_parent_folder_key_;

    CallbackType callback_;

    ResponseType response_;
    std::vector<ErrorType> errors_;

    push_type coro_{
            [this](pull_type & yield)
            {
                auto self = shared_from_this();  // Hold a reference to our
                // object until the coroutine
                // is complete, otherwise
                // handler will have invalid
                // reference to this because
                // the base object has
                // disappeared from scope

                std::function<void(const ResponseType & response)>
                        HandleMoveFolder =
                                [this, self](const ResponseType & response)
                {
                    if (response.error_code)
                    {
                        // If there was an error, insert into vector and
                        // propagate at the callback.

                        errors_.push_back(ErrorType(response.error_code,
                                                    response.error_string));
                    }
                    else
                    {
                        response_ = response;
                    }

                    // Resume the coroutine
                    (*this)();
                };

                stm_->Call(RequestType(folder_key_,
                                       destination_parent_folder_key_),
                           HandleMoveFolder);

                yield();

                // Coroutine is done, so call the callback.
                callback_(response_, errors_);
            }};
};

}  // namespace mf
}  // namespace api

#include "move_folder_impl.hpp"