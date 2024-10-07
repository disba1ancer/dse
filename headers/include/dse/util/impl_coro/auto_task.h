#ifndef DSE_UTIL_IMPL_CORO_AUTO_TASK_H
#define DSE_UTIL_IMPL_CORO_AUTO_TASK_H

#include "generic.h"
#include "../functional.h"

namespace dse::util {

namespace coroutine_impl {

struct suspend_destroy : std::suspend_always
{
    void await_suspend(std::coroutine_handle<> handle) const noexcept
    {
        handle.destroy();
    }
};

}

template <typename T>
struct auto_task {
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;
    using set_value_func = void(T);
    using set_value_ptr = FunctionPtr<set_value_func>;
    void start(set_value_ptr resultCallback)
    {
        auto& promise = _handle.promise();
        promise.set_value_func = resultCallback;
        std::exchange(_handle, nullptr).resume();
    }
private:
    auto_task(handle _handle) : _handle(_handle) {}
    coroutine_impl::non_copyable_coro_handle<promise_type> _handle;
};

template <typename T>
struct auto_task<T>::promise_type :
    coroutine_impl::return_unified_t<auto_task<T>::promise_type, T>
{
private:
    using task = auto_task<T>;
    friend task;
    static constexpr auto choice = coroutine_impl::type_index_v<T>;
public:
    task get_return_object() { return {task::handle::from_promise(*this)}; }
    auto initial_suspend() -> std::suspend_always { return {}; }
    auto final_suspend() noexcept -> coroutine_impl::suspend_destroy { return {}; }
    void return_unified()
    requires (choice == coroutine_impl::TypeChoice::Void)
    {
        set_value_func();
    }
    void return_unified(T&& val)
    requires (choice != coroutine_impl::TypeChoice::Void)
    {
        set_value_func(static_cast<decltype(val)>(val));
    }
    void return_unified(const T& val)
    requires (choice == coroutine_impl::TypeChoice::Value)
    {
        set_value_func(val);
    }
    void unhandled_exception()
    {
        std::terminate();
    }
private:
    task::set_value_ptr set_value_func;
};

} // namespace dse::util

#endif // DSE_UTIL_IMPL_CORO_AUTO_TASK_H
