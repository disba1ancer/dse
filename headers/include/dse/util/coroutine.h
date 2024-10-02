#ifndef DSE_UTIL_COROUTINE_H
#define DSE_UTIL_COROUTINE_H

#include <coroutine>
#include <exception>
#include <functional>
#include <variant>

namespace dse::util {

template <typename T>
struct task;

namespace coroutine_impl {

template <typename T>
class TaskAwaiter;

template <typename T>
struct FinalAwaiter;

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

template <typename T = void>
struct return_unified_base {
    using type = T;
};

template <>
struct return_unified_base<void> {
    using type = struct {};
};

template <typename T = void>
using return_unified_base_t = typename return_unified_base<T>::type;

template <typename D, typename T, typename B = void>
struct return_unified_t : return_unified_base_t<B> {
    void return_value(T&& v) {
        static_cast<D*>(this)->return_unified(std::forward<T>(v));
    }
    void return_value(const T& v) requires(!std::is_reference_v<T>) {
        static_cast<D*>(this)->return_unified(v);
    }
};

template <typename D, typename B>
struct return_unified_t<D, void, B> : return_unified_base_t<B> {
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

template <typename T>
struct task_owner {

};

} // namespace coroutine_impl

template <typename T>
struct task {
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;
    friend class coroutine_impl::TaskAwaiter<T>;
public:
    task(const task&) = delete;
    task(task&& task) noexcept :
        cHandle(nullptr)
    {
        *this = std::move(task);
    }
    task& operator =(const task&) = delete;
    task& operator =(task&& task) noexcept
    {
        std::swap(cHandle, task.cHandle);
        return *this;
    }
    ~task()
    {
        if (cHandle) cHandle.destroy();
    }
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
    handle cHandle;
    task(std::coroutine_handle<promise_type> handle) : cHandle(handle)
    {}
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

#endif // DSE_UTIL_COROUTINE_H
