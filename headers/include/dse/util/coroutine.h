#ifndef DSE_UTIL_COROUTINE_H
#define DSE_UTIL_COROUTINE_H

#include <coroutine>

#include <tuple>
#include <variant>
#include "execution.h"
#include "scope_exit.h"

namespace dse::util {

namespace impl {

template <typename Ret>
class SenderAwaiterRecv;

template <typename ... Tps>
class SenderAwaiterRecv<std::tuple<Tps...>> {
    std::coroutine_handle<> handle;
    std::tuple<Tps...>& value;
    std::exception_ptr& eptr;
public:
    SenderAwaiterRecv(std::tuple<Tps...>& value, std::exception_ptr& eptr, std::coroutine_handle<> handle) : handle(handle), value(value), eptr(eptr)
    {}
    template <typename ... Tps2>
    requires (sizeof... (Tps) == sizeof... (Tps2))
    friend void dse_TagInvoke(
        TagT<SetValue>,
        SenderAwaiterRecv<std::tuple<Tps...>>&& recv,
        Tps2&& ... args
    ) {
        recv.value = std::make_tuple(std::forward<Tps2>(args)...);
        recv.handle.resume();
    }
    friend void dse_TagInvoke(TagT<SetDone>, SenderAwaiterRecv&& recv)
    {
        recv.handle.resume();
    }
    friend void dse_TagInvoke(TagT<SetError>, SenderAwaiterRecv&& recv, std::exception_ptr&& eptr)
    {
        recv.eptr = eptr;
        recv.handle.resume();
    }
};

//template <typename ... Tps>
//void dse_TagInvoke(
//    TagT<SetValue>,
//    SenderAwaiterRecv<std::tuple<std::remove_reference_t<Tps>...>>&& recv,
//    Tps&& ... args
//) {
//    recv.value = std::make_tuple(std::forward<Tps>(args)...);
//    recv.handle.resume();
//}

template <typename T>
struct ReturnFromTuple {
    using Type = T;
};

template <template<typename ...> typename T>
struct ReturnFromTuple<T<>> {
    using Type = void;
};

template <template<typename ...> typename T, typename Single>
struct ReturnFromTuple<T<Single>> {
    using Type = Single;
};

template <typename T>
using ReturnFromTupleT = typename ReturnFromTuple<T>::Type;

template <typename S>
requires (
    std::same_as<typename SenderTraits<S>::ErrorType, std::exception_ptr>
)
class SenderAwaiter {
    using ReturnedValue = typename SenderTraits<S>::template ValueTypes<std::tuple>;
    using OpState = TagInvokeResultT<TagT<Connect>, std::remove_cvref_t<S>&&, SenderAwaiterRecv<ReturnedValue>&&>;
    std::variant<std::remove_cvref_t<S>, OpState> sndropstate;
    ReturnedValue value;
    std::exception_ptr eptr;
public:
    SenderAwaiter(S&& sndr) : sndropstate(std::forward<S>(sndr))
    {}
    bool await_ready()
    { return false; }
    void await_suspend(std::coroutine_handle<> handle)
    {
        auto sndr = std::get<std::remove_cvref_t<S>>(sndropstate);
        sndropstate.template emplace<OpState>(Connect(std::move(sndr), SenderAwaiterRecv<ReturnedValue>(value, eptr, handle)));
        Start(std::get<OpState>(sndropstate));
    }
    ReturnFromTupleT<ReturnedValue> await_resume()
    {
        if (eptr) {
            std::rethrow_exception(eptr);
        }
        if constexpr (std::tuple_size_v<ReturnedValue> > 1) {
            return std::move(value);
        } else if constexpr (std::tuple_size_v<ReturnedValue> == 1) {
            return std::move(std::get<0>(value));
        } else {
            return;
        }
    }
};

}

template <typename T>
struct Task;

namespace impl {

template <typename T>
struct TaskSpec;

template <typename T>
class TaskAwaiter;

template <typename Ret, typename Recv>
class TaskOpstate;

template <typename T>
struct TaskBase;

template <typename T>
struct FinalAwaiter;

template <typename T>
struct CoroReceiver {
    virtual void SetValue(T&&) = 0;
    virtual void SetError(std::exception_ptr&& e) = 0;
    virtual void SetDone() = 0;
};

template <>
struct CoroReceiver<void> {
    virtual void SetValue() = 0;
    virtual void SetError(std::exception_ptr&& e) = 0;
    virtual void SetDone() = 0;
};

template <typename T>
struct PromiseTypeBase {
    friend TaskSpec<T>;
    friend TaskBase<T>;
    friend FinalAwaiter<T>;
private:
    std::variant<std::monostate, T, std::exception_ptr> value;
public:
    void return_value(const T& val)
    {
        value.template emplace<1>(val);
    }
    void SetValueForRecv(CoroReceiver<T>* recv)
    {
        recv->SetValue(std::move(std::get<1>(value)));
    }
};

template <>
struct PromiseTypeBase<void> {
    friend TaskSpec<void>;
    friend TaskBase<void>;
    friend FinalAwaiter<void>;
private:
    std::variant<std::monostate, std::monostate, std::exception_ptr> value;
public:
    void return_void()
    {
        value.template emplace<1>();
    }
    void SetValueForRecv(CoroReceiver<void>* recv)
    {
        recv->SetValue();
    }
};

template <typename T>
struct PromiseTypeBase<T&> {
    friend TaskSpec<T&>;
    friend TaskBase<T&>;
    friend FinalAwaiter<T&>;
private:
    std::variant<std::monostate, std::reference_wrapper<T>, std::exception_ptr> value;
public:
    void return_value(T& val)
    {
        value.template emplace<1>(std::ref(val));
    }
    void SetValueForRecv(CoroReceiver<T&>* recv)
    {
        recv->SetValue(static_cast<T&>(std::get<1>(value).get()));
    }
};

template <typename T>
struct PromiseTypeBase<T&&> {
    friend TaskSpec<T&&>;
    friend TaskBase<T&&>;
    friend FinalAwaiter<T&&>;
private:
    std::variant<std::monostate, std::reference_wrapper<T>, std::exception_ptr> value;
public:
    void return_value(T&& val)
    {
        value.template emplace<1>(std::ref(val));
    }
    void SetValueForRecv(CoroReceiver<T&&>* recv)
    {
        recv->SetValue(static_cast<T&&>(std::get<1>(value).get()));
    }
};

template <typename T>
struct TaskBase {
    struct PromiseType;
    using Handle = std::coroutine_handle<PromiseType>;
    friend class TaskAwaiter<T>;
    template <typename Ret, typename Recv>
    friend void dse_TagInvoke(TagT<Start>, TaskOpstate<Ret, Recv>&);
protected:
    Handle handle;
    TaskBase(std::coroutine_handle<PromiseType> handle) : handle(handle)
    {}
    void ThrowIfExcept()
    {
        auto& promise = handle.promise();
        if (promise.value.index() == 2) {
            std::rethrow_exception(std::get<2>(promise.value));
        }
    }
public:
    TaskBase(const TaskBase&) = delete;
    TaskBase(TaskBase&& task) noexcept :
        handle(task.handle)
    {
        task.handle = nullptr;
    }
    TaskBase& operator =(const TaskBase&) = delete;
    TaskBase& operator =(TaskBase&& task) noexcept
    {
        std::swap(handle, task.handle);
    }
    ~TaskBase()
    {
        if (handle) handle.destroy();
    }
    void Resume() {
        handle.resume();
    }
    void operator()() {
        handle();
    }
    void ResumeDetached() {
        auto h = std::exchange(handle, {});
        h.promise().MakeDetached();
        h.resume();
    }
    bool Done() {
        return handle.done();
    }
};

template <typename T>
struct TaskBase<T>::PromiseType : PromiseTypeBase<T> {
    friend class TaskAwaiter<T>;
    friend struct FinalAwaiter<T>;
    template <typename Ret, typename Recv>
    friend void dse_TagInvoke(TagT<Start>, TaskOpstate<Ret, Recv>&);
    auto get_return_object() -> Task<T>
    {
        return { TaskSpec<T>::Handle::from_promise(*this) };
    }
    auto initial_suspend() -> std::suspend_always
    {
        return {};
    }
    void unhandled_exception()
    {
        this->value.template emplace<2>(std::current_exception());
    }
    auto final_suspend() noexcept -> FinalAwaiter<T>
    {
        return {};
    }
    void MakeDetached()
    {
        detached = true;
    }
private:
    std::variant<std::monostate, std::coroutine_handle<>, CoroReceiver<T>*> rsm;
    bool detached = false;
};

template <typename T>
struct FinalAwaiter {
    bool await_ready() noexcept
    {
        return false;
    }
    auto await_suspend(std::coroutine_handle<typename TaskBase<T>::PromiseType> handle) noexcept
    -> std::coroutine_handle<>
    {
        auto& promise = handle.promise();
        scope_exit final = [&promise, &handle]{
            if (promise.detached) {
                handle.destroy();
            }
        };
        switch (promise.rsm.index()) {
            case 1:
                return std::get<1>(promise.rsm);
            case 2: {
                auto recv = std::get<2>(promise.rsm);
                switch (promise.value.index()) {
                    case 2:
                        recv->SetError(std::move(std::get<2>(promise.value)));
                        break;
                    case 1:
                        try {
                            promise.SetValueForRecv(recv);
                        } catch (...) {
                            recv->SetError(std::move(std::current_exception()));
                        }
                        [[fallthrough]];
                    default:
                        break;
                }
            } [[fallthrough]];
            default:
                break;
        }
        return std::noop_coroutine();
    }
    void await_resume() noexcept
    {}
};

template <typename T>
struct TaskSpec : TaskBase<T> {
    using TaskBase = TaskBase<T>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    TaskSpec(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    T Result()
    {
        this->ThrowIfExcept();
        return std::move(std::get<1>(this->handle.promise().value));
    }
};

template <>
struct TaskSpec<void> : TaskBase<void> {
    using TaskBase = TaskBase<void>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    TaskSpec(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    void Result()
    {
        this->ThrowIfExcept();
    }
};

template <typename T>
struct TaskSpec<T&> : TaskBase<T&> {
    using TaskBase = TaskBase<T&>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    TaskSpec(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    T& Result()
    {
        this->ThrowIfExcept();
        return std::get<1>(this->handle.promise().value);
    }
};

template <typename T>
struct TaskSpec<T&&> : TaskBase<T&&> {
    using TaskBase = TaskBase<T&&>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    TaskSpec(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    T&& Result()
    {
        this->ThrowIfExcept();
        return static_cast<T&&>(std::get<1>(this->handle.promise().value).get());
    }
};

}

template <typename T>
struct Task : impl::TaskSpec<T> {
    Task(std::coroutine_handle<typename impl::TaskSpec<T>::promise_type> handle) :
        impl::TaskSpec<T>(handle)
    {}
    friend void dse_TagInvoke(TagT<StartDetached>, Task<T>&& task)
    {
        task.ResumeDetached();
    }

    template <typename Recv>
    auto dse_TagInvoke(util::TagT<util::Connect>, Task&& sndr, Recv&& recv)
    -> impl::TaskOpstate<T, Recv>
    {
        return { std::move(sndr), std::forward<Recv>(recv) };
    }
};

namespace impl {

template <typename T>
class TaskAwaiter {
    Task<T>& task;
public:
    TaskAwaiter(Task<T>& task) :
        task(task)
    {}
    auto await_ready() -> bool {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> handle) -> std::coroutine_handle<> {
        auto& promise = task.handle.promise();
        promise.rsm = handle;
        return task.handle;
    }
    T await_resume()
    {
        return task.Result();
    }
};

}

template <typename T>
auto operator co_await(Task<T>&& task) -> impl::TaskAwaiter<T>
{
    return impl::TaskAwaiter(task);
}

template <typename T>
auto operator co_await(Task<T>& task) -> impl::TaskAwaiter<T>
{
    return impl::TaskAwaiter(task);
}

template <typename T>
struct SenderTraits<Task<T>> {
    template <template<typename ...> typename TT>
    using ValueTypes = TT<T>;
    using ErrorType = std::exception_ptr;
    static constexpr bool SendsDone = false;
};

namespace impl {

template <typename Ret, typename Recv>
struct TaskOpstateBase : CoroReceiver<Ret> {
    Task<Ret> task;
    Recv receiver;
    void SetValue(Ret&& val) override
    {
        util::SetValue(std::move(this->receiver), val);
    }
public:
    TaskOpstateBase(Task<Ret>&& task, Recv&& receiver) :
        task(std::move(task)),
        receiver(std::forward<Recv>(receiver))
    {}
};

template <typename Recv>
struct TaskOpstateBase<void, Recv> : CoroReceiver<void> {
    Task<void> task;
    Recv receiver;
    void SetValue() override
    {
        util::SetValue(std::move(this->receiver));
    }
public:
    TaskOpstateBase(Task<void>&& task, Recv&& receiver) :
        task(std::move(task)),
        receiver(std::forward<Recv>(receiver))
    {}
};

template <typename Ret, typename Recv>
class TaskOpstate : TaskOpstateBase<Ret, Recv> {
    void SetError(std::exception_ptr&& e) override
    {
        util::SetError(std::move(this->receiver), std::move(e));
    }
    void SetDone() override
    {}
public:
    TaskOpstate(Task<Ret>&& task, Recv&& receiver) :
        TaskOpstateBase<Ret, Recv>(std::move(task), std::forward<Recv>(receiver))
    {}
    friend void dse_TagInvoke(util::TagT<util::Start>, TaskOpstate<Ret, Recv>& sndr) {
        auto handle = sndr.task.handle;
        auto& promise = handle.promise();
        promise.rsm = static_cast<CoroReceiver<Ret>*>(&sndr);
        handle.resume();
    }
};

}

struct CurrentHandle {
    auto await_ready() -> bool {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> h) -> std::coroutine_handle<> {
        handle = h;
        return h;
    }
    auto await_resume() -> std::coroutine_handle<>
    {
        return handle;
    }
private:
    std::coroutine_handle<> handle;
};

}

template <dse::util::Sender S>
auto operator co_await(S&& sndr) -> dse::util::impl::SenderAwaiter<S>
{
    return { std::forward<S>(sndr) };
}

#endif // DSE_UTIL_COROUTINE_H
