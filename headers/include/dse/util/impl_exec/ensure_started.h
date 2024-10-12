#ifndef ENSURE_STARTED_H
#define ENSURE_STARTED_H

#include <concepts>
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
auto ensure_started_coro(eager_task_t, A& a)
-> ::std::future<decltype(call_co_await<void>(a).await_ready())>
{
    co_return co_await a;
}

template <awaiter<> A>
auto ensure_started_coro(eager_task_t, A&& a)
-> ::std::future<decltype(call_co_await<void>(a).await_ready())>
{
    auto a2 = std::move(a);
    co_return co_await a2;
}

template <awaiter<> A>
auto ensure_started(A&& a)
-> ::std::future<decltype(call_co_await<void>(a).await_ready())>
{
    return ensure_started_coro({}, std::forward<A>(a));
}

} // namespace dse::util::ensure_started_impl

#endif // ENSURE_STARTED_H
