#ifndef DSE_UTIL_IMPL_CORO_TASK_H
#define DSE_UTIL_IMPL_CORO_TASK_H

#include "generic.h"

namespace dse::util {

template <typename T>
struct task;

namespace coroutine_impl {

template <typename T>
class TaskAwaiter;

template <typename T>
struct FinalAwaiter {
    bool await_ready() noexcept
    {
        return false;
    }
    auto await_suspend(std::coroutine_handle<typename task<T>::promise_type> handle) noexcept
    -> std::coroutine_handle<>
    {
        auto& promise = handle.promise();
        if (promise.rsm == nullptr) {
            return std::noop_coroutine();
        }
        return promise.rsm;
    }
    void await_resume() noexcept
    {}
};

} // namespace coroutine_impl

template <typename T>
struct task {
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;
    friend class coroutine_impl::TaskAwaiter<T>;
public:
    bool done() {
        return cHandle.done();
    }
    auto operator co_await() && -> coroutine_impl::TaskAwaiter<T>
    {
        return {*this};
    }
    auto operator co_await() & -> coroutine_impl::TaskAwaiter<T>
    {
        return {*this};
    }
private:
    static constexpr auto choice = coroutine_impl::type_index_v<T>;
    coroutine_impl::non_copyable_coro_handle<promise_type> cHandle;
    task(handle h) : cHandle(h) {}
};

template <typename T>
struct task<T>::promise_type :
        public coroutine_impl::return_unified_t<task<T>::promise_type, T>
{
    friend coroutine_impl::FinalAwaiter<T>;
    friend class coroutine_impl::TaskAwaiter<T>;
    friend struct task<T>;
private:
    using type = std::conditional_t<task::choice == coroutine_impl::TypeChoice::Void, std::monostate, T>;
public:
    auto get_return_object() -> task<T>
    {
        return { task<T>::handle::from_promise(*this) };
    }
    auto initial_suspend() -> std::suspend_always
    {
        return {};
    }
    void return_unified()
    requires(task::choice == coroutine_impl::TypeChoice::Void)
    {
        value.template emplace<1>();
    }
    void return_unified(type val)
    requires(task::choice == coroutine_impl::TypeChoice::Reference)
    {
        value.template emplace<1>(std::ref(val));
    }
    void return_unified(type&& val)
    requires(task::choice == coroutine_impl::TypeChoice::Value)
    {
        value.template emplace<1>(std::move(val));
    }
    void return_unified(const type& val)
    requires(task::choice == coroutine_impl::TypeChoice::Value)
    {
        value.template emplace<1>(val);
    }
    void unhandled_exception()
    {
        this->value.template emplace<2>(std::current_exception());
    }
    auto final_suspend() noexcept -> coroutine_impl::FinalAwaiter<T>
    {
        return {};
    }
private:
    void ThrowIfExcept()
    {
        if (value.index() == 2) {
            std::rethrow_exception(std::get<2>(value));
        }
    }
    void Result()
        requires(task::choice == coroutine_impl::TypeChoice::Void)
    {
        this->ThrowIfExcept();
    }
    T Result()
        requires(task::choice == coroutine_impl::TypeChoice::Value)
    {
        this->ThrowIfExcept();
        return std::move(std::get<1>(value));
    }
    T Result()
        requires(task::choice == coroutine_impl::TypeChoice::Reference)
    {
        this->ThrowIfExcept();
        return static_cast<T>(std::get<1>(value).get());
    }
private:
    using ref_wrapper = std::reference_wrapper<std::remove_reference_t<T>>;
    using storage_type = coroutine_impl::select_type_t<int(choice), std::monostate, ref_wrapper, T>;
    std::variant<std::monostate, storage_type, std::exception_ptr> value;
    std::coroutine_handle<> rsm;
};

namespace coroutine_impl {

template <typename T>
class TaskAwaiter {
    task<T>& coro;
public:
    TaskAwaiter(task<T>& coro) :
        coro(coro)
    {}
    auto await_ready() -> bool {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> handle) -> std::coroutine_handle<> {
        auto& promise = coro.cHandle.promise();
        promise.rsm = handle;
        return coro.cHandle;
    }
    T await_resume()
    {
        return coro.cHandle.promise().Result();
    }
};

} // namespace coroutine_impl

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

} // namespace dse::util

#endif // DSE_UTIL_IMPL_CORO_TASK_H
