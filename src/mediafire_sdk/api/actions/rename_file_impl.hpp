namespace mf
{
namespace api
{

template <class TRequest>
RenameFile<TRequest>::ErrorType::ErrorType(const std::string & quick_key,
                                           const std::string & new_name,
                                           const std::error_code & error_code,
                                           const std::string & error_string)
        : quick_key(quick_key),
          new_name(new_name),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<RenameFile<TRequest>> RenameFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & quick_key,
        const std::string & new_name,
        CallbackType && callback)
{
    return std::shared_ptr<RenameFile>(
            new RenameFile(stm, quick_key, new_name, std::move(callback)));
}

template <class TRequest>
RenameFile<TRequest>::RenameFile(SessionMaintainer * stm,
                                 const std::string & quick_key,
                                 const std::string & new_name,
                                 CallbackType && callback)
        : stm_(stm),
          quick_key_(quick_key),
          new_name_(new_name),
          callback_(callback)
{
}

template <class TRequest>
void RenameFile<TRequest>::Cancel()
{
    cancelled_ = true;

    if (request_ != nullptr)
        request_->Cancel();
}

template <class TRequest>
void RenameFile<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleFileGetInfo =
            [this, self](const ResponseType & response)
    {
        if (response.error_code)
        {
            // If there was an error, insert into vector and
            // propagate at the callback.
            std::string error_str = "No error string provided";
            if (response.error_string)
                error_str = *response.error_string;

            errors_.push_back(ErrorType(
                    quick_key_, new_name_, response.error_code, error_str));
        }
        else
        {
            response_ = response;
        }

        // Resume the coroutine

        request_ = nullptr;  // Must free request_ or coroutine cannot be
        // destructed.
        Resume();
    };

    request_ = stm_->Call(RequestType(quick_key_), HandleFileGetInfo);

    if (cancelled_)
        request_->Cancel();

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}
}  // namespace mf
}  // namespace api