#pragma once

#include <memory>

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

class Coroutine : public boost::coroutines::coroutine<void>,
                  public std::enable_shared_from_this<Coroutine>
{
private:
    friend struct CoroutineWorkManagerAttorney;

private:
    using WorkManagerCompletionHandlerType = std::function<void()>;

protected:
    virtual ~Coroutine() {}

public:
    virtual void Start() { coro_(); }

    virtual void Resume() { coro_(); }

    virtual void Cancel() = 0;

private:
    virtual void CoroutineBody(pull_type & yield) = 0;

private:
    push_type coro_{[this](pull_type & yield)
                    {
                        /* Keep self alive until completion.  Must be created
                         * before first call to "yield". */
                        auto self = shared_from_this();

                        self->CoroutineBody(yield);

                        self->WorkManagerCompletionHandler();

                        self->WorkManagerCompletionHandler = [](){};
                    }};

    WorkManagerCompletionHandlerType WorkManagerCompletionHandler = [](){};

protected:
    bool cancelled_ = false;

};

struct CoroutineWorkManagerAttorney
{
    static void SetCompletionHandler(
            std::shared_ptr<Coroutine> coro,
            Coroutine::WorkManagerCompletionHandlerType CompletionHandler)
    {
        coro->WorkManagerCompletionHandler = std::move(CompletionHandler);
    }
};

}  // namespace api
}  // namespace mf
