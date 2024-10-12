#ifndef DSE_UTIL_IMPL_CORO_EAGER_TASK_H
#define DSE_UTIL_IMPL_CORO_EAGER_TASK_H

#include <future>
#include "generic.h"

namespace dse::util {

template <typename T>
struct eager_task
{
    struct promise_type;
    using handle_t = ::std::coroutine_handle<promise_type>;
};

template <typename T>
struct eager_task<T>::promise_type :
    ::std::promise<T>,
    coroutine_impl::return_unified_t<promise_type, T>
{
    using ::std::promise<T>::set_value;
    using ::std::promise<T>::set_exception;
    static constexpr auto choice = coroutine_impl::type_index_v<T>;
    auto get_return_object() -> eager_task { return {*this, }; }
    auto initial_suspend() -> ::std::suspend_never { return {}; }
    struct final_awaiter : ::std::suspend_always
    {
        void await_suspend(::std::coroutine_handle<> handle) noexcept
        {
            handle.destroy();
        }
    };

    auto final_suspend() noexcept -> final_awaiter { return {}; }
    void return_unified()
    requires(choice == coroutine_impl::TypeChoice::Void)
    {
        set_value();
    }
    void return_unified(T&& r)
    requires(choice != coroutine_impl::TypeChoice::Void)
    {
        set_value(r);
    }
    void return_unified(const T& r)
    requires(choice == coroutine_impl::TypeChoice::Value)
    {
        set_value(r);
    }
    void unhandled_exception() { set_exception(std::current_exception()); }
};

} // namespace dse::util

#endif // DSE_UTIL_IMPL_CORO_EAGER_TASK_H
