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
class TaskAwaiter;

template <typename Ret, typename Recv>
class TaskOpstate;

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

enum class TypeChoice {
    Void,
    Reference,
    Value
};

template <typename T>
struct type_index {
private:
    static constexpr auto T_is_void = std::same_as<T, void>;
    static constexpr auto T_is_reference = std::is_reference_v<T>;
public:
    static constexpr auto value = []() constexpr -> TypeChoice {
        if (T_is_void) {
            return TypeChoice::Void;
        }
        if (T_is_reference) {
            return TypeChoice::Reference;
        }
        return TypeChoice::Value;
    }();
};

template <typename T>
constexpr auto type_index_v = type_index<T>::value;

template <int n, typename ... Types>
struct select_type {
private:
    using typelist = std::variant<Types...>;
public:
    using type = std::variant_alternative_t<n, typelist>;
};

template <int n, typename ... Types>
using select_type_t = select_type<n, Types...>::type;

template <typename D, typename T>
struct return_unified_t {
    void return_value(T&& v) {
        static_cast<D*>(this)->return_unified(std::forward<T>(v));
    }
};

template <typename D>
struct return_unified_t<D, void> {
    void return_void() {
        static_cast<D*>(this)->return_unified();
    }
};

template <typename T>
struct FinalAwaiter {
    bool await_ready() noexcept
    {
        return false;
    }
    auto await_suspend(std::coroutine_handle<typename Task<T>::promise_type> handle) noexcept
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

}

template <typename T>
struct Task {
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;
    friend class impl::TaskAwaiter<T>;
    template <typename Ret, typename Recv>
    friend void impl::dse_TagInvoke(TagT<Start>, impl::TaskOpstate<Ret, Recv>&);
private:
    static constexpr auto choice = impl::type_index_v<T>;
    handle cHandle;
    Task(std::coroutine_handle<promise_type> handle) : cHandle(handle)
    {}
    void ThrowIfExcept()
    {
        auto& promise = cHandle.promise();
        if (promise.value.index() == 2) {
            std::rethrow_exception(std::get<2>(promise.value));
        }
    }
public:
    Task(const Task&) = delete;
    Task(Task&& task) noexcept :
        cHandle(nullptr)
    {
        *this = std::move(task);
    }
    Task& operator =(const Task&) = delete;
    Task& operator =(Task&& task) noexcept
    {
        std::swap(cHandle, task.cHandle);
    }
    ~Task()
    {
        if (cHandle) cHandle.destroy();
    }
    void Resume() {
        cHandle.resume();
    }
    void operator()() {
        cHandle();
    }
    void ResumeDetached() {
        auto h = std::exchange(cHandle, {});
        h.promise().MakeDetached();
        h.resume();
    }
    bool Done() {
        return cHandle.done();
    }
    void Result()
    requires(promise_type::choice == impl::TypeChoice::Void)
    {
        this->ThrowIfExcept();
    }
    T Result()
    requires(promise_type::choice == impl::TypeChoice::Value)
    {
        this->ThrowIfExcept();
        return std::move(std::get<1>(cHandle.promise().value));
    }
    T Result()
    requires(promise_type::choice == impl::TypeChoice::Reference)
    {
        this->ThrowIfExcept();
        return static_cast<T>(std::get<1>(cHandle.promise().value).get());
    }
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

template <typename T>
struct Task<T>::promise_type :
    public impl::return_unified_t<Task<T>::promise_type, T>
{
    friend impl::FinalAwaiter<T>;
    friend class impl::TaskAwaiter<T>;
    template <typename Ret, typename Recv>
    friend void impl::dse_TagInvoke(TagT<Start>, impl::TaskOpstate<Ret, Recv>&);
private:
    using type = std::conditional_t<Task::choice == impl::TypeChoice::Void, std::monostate, T>;
public:
    auto get_return_object() -> Task<T>
    {
        return { Task<T>::handle::from_promise(*this) };
    }
    auto initial_suspend() -> std::suspend_always
    {
        return {};
    }
    void return_unified()
    requires(Task::choice == impl::TypeChoice::Void)
    {
        value.template emplace<1>();
    }
    void return_unified(type val)
    requires(Task::choice == impl::TypeChoice::Reference)
    {
        value.template emplace<1>(std::ref(val));
    }
    void return_unified(type&& val)
    requires(Task::choice == impl::TypeChoice::Value)
    {
        value.template emplace<1>(std::move(val));
    }
    void unhandled_exception()
    {
        this->value.template emplace<2>(std::current_exception());
    }
    auto final_suspend() noexcept -> impl::FinalAwaiter<T>
    {
        return {};
    }
    void SetValueForRecv(impl::CoroReceiver<T>* recv)
    requires(Task::choice == impl::TypeChoice::Void)
    {
        recv->SetValue();
    }
    void SetValueForRecv(impl::CoroReceiver<T>* recv)
    requires(Task::choice == impl::TypeChoice::Reference)
    {
        recv->SetValue(static_cast<T>(std::get<1>(value).get()));
    }
    void SetValueForRecv(impl::CoroReceiver<T>* recv)
    requires(Task::choice == impl::TypeChoice::Value)
    {
        recv->SetValue(std::move(std::get<1>(value)));
    }
    void MakeDetached()
    {
        detached = true;
    }
private:
    using ref_wrapper = std::reference_wrapper<std::remove_reference_t<T>>;
    using storage_type = impl::select_type_t<int(choice), std::monostate, ref_wrapper, T>;
    std::variant<std::monostate, storage_type, std::exception_ptr> value;
    std::variant<std::monostate, std::coroutine_handle<>, impl::CoroReceiver<T>*> rsm;
    bool detached = false;
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
        auto& promise = task.cHandle.promise();
        promise.rsm = handle;
        return task.cHandle;
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
        auto handle = sndr.task.cHandle;
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
