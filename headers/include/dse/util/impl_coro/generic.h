#ifndef DSE_UTIL_IMPL_CORO_GENERIC_H
#define DSE_UTIL_IMPL_CORO_GENERIC_H

#include <concepts>
#include <variant>
#include <coroutine>

namespace dse::util::coroutine_impl {

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
    template <typename U>
    requires requires (D d, U&& v) {
        d.return_unified(static_cast<decltype(v)>(v));
    }
    void return_value(U&& v) {
        static_cast<D*>(this)->return_unified(static_cast<decltype(v)>(v));
    }
};

template <typename D, typename B>
struct return_unified_t<D, void, B> : return_unified_base_t<B> {
    void return_void() {
        static_cast<D*>(this)->return_unified();
    }
};

template <typename Promise>
struct non_copyable_coro_handle : std::coroutine_handle<Promise> {
    using handle = std::coroutine_handle<Promise>;
    non_copyable_coro_handle(handle _handle) : handle(_handle) {}
    non_copyable_coro_handle(const non_copyable_coro_handle&) = delete;
    non_copyable_coro_handle(non_copyable_coro_handle &&h) : handle(nullptr)
    {
        *this = static_cast<handle&&>(h);
    }
    using handle::operator=;

    non_copyable_coro_handle &
    operator=(const non_copyable_coro_handle &) = delete;

    non_copyable_coro_handle& operator=(non_copyable_coro_handle&& h)
    {
        std::swap<handle>(*this, h);
        return *this;
    }
    ~non_copyable_coro_handle()
    {
        if (*this) handle::destroy();
    }
};

} // namespace dse::util::coroutine_impl

#endif // DSE_UTIL_IMPL_CORO_GENERIC_H
