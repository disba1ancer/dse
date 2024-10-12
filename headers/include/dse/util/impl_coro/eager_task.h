#ifndef DSE_UTIL_IMPL_CORO_EAGER_TASK_H
#define DSE_UTIL_IMPL_CORO_EAGER_TASK_H

#include <future>
#include "generic.h"

namespace dse::util {

template <typename T>
struct eager_promise :
    coroutine_impl::return_unified_t<eager_promise<T>, T>,
    private ::std::promise<T>
{
private:
    using ::std::promise<T>::get_future;
    using ::std::promise<T>::set_value;
    using ::std::promise<T>::set_exception;
    static constexpr auto choice = coroutine_impl::type_index_v<T>;
    using type = coroutine_impl::replace_void_t<T>;
public:
    auto get_return_object() -> ::std::future<T> { return get_future(); }
    auto initial_suspend() -> ::std::suspend_never { return {}; }
    auto final_suspend() noexcept -> coroutine_impl::suspend_destroy
    {
        return {};
    }
    void return_unified()
    requires(choice == coroutine_impl::TypeChoice::Void)
    {
        set_value();
    }
    void return_unified(type&& r)
    requires(choice != coroutine_impl::TypeChoice::Void)
    {
        set_value(r);
    }
    void return_unified(const type& r)
    requires(choice == coroutine_impl::TypeChoice::Value)
    {
        set_value(r);
    }
    void unhandled_exception() { set_exception(std::current_exception()); }
};

struct eager_task_t {};

} // namespace dse::util

namespace std {

template <typename T, typename ... Args>
struct coroutine_traits<future<T>, ::dse::util::eager_task_t, Args...>
{
    using promise_type = ::dse::util::eager_promise<T>;
};

template <typename T, typename A0, typename ... Args>
struct coroutine_traits<future<T>, A0, ::dse::util::eager_task_t, Args...>
{
    using promise_type = ::dse::util::eager_promise<T>;
};

} // namespace std

#endif // DSE_UTIL_IMPL_CORO_EAGER_TASK_H
