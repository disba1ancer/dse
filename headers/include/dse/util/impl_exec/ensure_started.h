#ifndef ENSURE_STARTED_H
#define ENSURE_STARTED_H

#include <future>
#include <concepts>
#include <coroutine>
#include "../coroutine.h"

namespace dse::util::ensure_started_impl {

template <typename A>
concept awaitable_base = requires(A a) {
    {
        a.await_ready()
    } -> std::same_as<bool>;
    a.await_resume();
};

template <typename T>
struct is_await_suspend_result_t : ::std::false_type
{};

template <>
struct is_await_suspend_result_t<void> : ::std::true_type
{};

template <>
struct is_await_suspend_result_t<bool> : ::std::true_type
{};

template <typename T>
struct is_await_suspend_result_t<::std::coroutine_handle<T>> : ::std::true_type
{};

template <typename T>
concept is_await_suspend_result = is_await_suspend_result_t<T>::value;

template <typename A, typename P = void>
concept awaitable_result = requires(A a, ::std::coroutine_handle<P> p) {
    {
        a.await_suspend(p)
    } -> is_await_suspend_result;
};

template <typename A, typename P = void>
concept awaitable = awaitable_base<::std::remove_reference_t<A>> && awaitable_result<::std::remove_reference_t<A>, P>;

template <typename A, typename P = void>
concept awaiter = requires(A a){
    { operator co_await(a) } -> awaitable<P>;
} || requires(A a){
    { a.operator co_await() } -> awaitable<P>;
};

template <typename T>
bool suspend(T handle)
{
    handle.resume();
    return true;
}

template <>
inline bool suspend<bool>(bool result)
{
    return result;
}

inline bool suspend(void)
{
    return true;
}

template <typename R>
struct future_task
{
    struct promise_type;
    using handle_t = ::std::coroutine_handle<promise_type>;
    struct promise_type :
        ::std::promise<R>,
        coroutine_impl::return_unified_t<promise_type, R>
    {
        using ::std::promise<R>::set_value;
        using ::std::promise<R>::set_exception;
        auto get_return_object() -> future_task { return {*this}; }
        auto initial_suspend() -> ::std::suspend_never { return {}; }
        struct final_awaiter : ::std::suspend_always
        {
            using suspend_always::await_ready;
            void await_suspend(::std::coroutine_handle<> handle) noexcept
            {
                handle.destroy();
            }
            using suspend_always::await_resume;
        };

        auto final_suspend() noexcept -> final_awaiter { return {}; }
        void return_unified()
            requires(::std::same_as<R, void>)
        {
            set_value();
        }
        void return_unified(R r)
            requires(::std::is_reference_v<R>)
        {
            set_value(r);
        }
        void return_unified(R&& r) { set_value(r); }
        void unhandled_exception() { set_exception(std::current_exception()); }
    };
    auto get_future() -> ::std::future<R> { return promise.get_future(); }

private:
    future_task(promise_type& promise) :
        promise(promise)
    {}
    promise_type& promise;
};

template <typename P, typename A>
requires requires(A a){
    { operator co_await(a) } -> awaitable<P>;
}
auto call_co_await(A&& a) -> decltype(operator co_await(a))
{
    return operator co_await(a);
}

template <typename P, typename A>
requires requires(A a){
    { a.operator co_await() } -> awaitable<P>;
}
auto call_co_await(A&& a) -> decltype(a.operator co_await())
{
    return a.operator co_await();
}

template <awaiter<> A>
auto ensure_started_coro(A& a)
-> future_task<decltype(call_co_await<void>(a).await_ready())>
{
    co_return co_await a;
}

template <awaiter<> A>
auto ensure_started_coro(A&& a)
-> future_task<decltype(call_co_await<void>(a).await_ready())>
{
    auto a2 = std::move(a);
    co_return co_await a2;
}

template <awaiter<> A>
auto ensure_started(A&& a)
-> ::std::future<decltype(call_co_await<void>(a).await_ready())>
{
    return ensure_started_coro(std::forward<A>(a)).get_future();
}

} // namespace dse::util::ensure_started_impl

#endif // ENSURE_STARTED_H
