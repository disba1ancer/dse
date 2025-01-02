/*
 * enum_bitwise.h
 *
 *  Created on: 12 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_UTIL_ENUM_BITWISE_H_
#define DSE_UTIL_ENUM_BITWISE_H_

#include <type_traits>
#include <utility>
#include <concepts>

namespace dse::util {

template <typename T>
requires std::is_enum_v<T>
struct enum_bit_ops {};

namespace enum_bit_ops_impl {

template <typename T = void>
concept bit_ops_enabled = requires (enum_bit_ops<T> t) { enable(t); };

struct Bool
{
    Bool(bool val) : val(val) {}
    operator bool() { return val; }
    bool val;
};

}

} // namespace dse::util

template <dse::util::enum_bit_ops_impl::bit_ops_enabled T>
auto operator+(const T& a) -> std::underlying_type_t<T>
{
    return std::to_underlying(a);
}

#define GENERATE(op) \
template <dse::util::enum_bit_ops_impl::bit_ops_enabled T> \
T operator op(const T& a, const T& b) \
{ \
    return static_cast<T>(+a op +b); \
} \
template <dse::util::enum_bit_ops_impl::bit_ops_enabled T> \
T& operator op##=(T& a, const T& b) \
{ \
    a = a op b; \
    return a; \
}

GENERATE(|)
GENERATE(&)
GENERATE(^)
#undef GENERATE

template <dse::util::enum_bit_ops_impl::bit_ops_enabled T>
T operator~(const T& a)
{
    return static_cast<T>(~+a);
}

template <dse::util::enum_bit_ops_impl::bit_ops_enabled T>
T operator-(const T& a, const T& b)
{
    return a & b ^ a;
}

template <dse::util::enum_bit_ops_impl::bit_ops_enabled T>
T& operator-=(T& a, const T& b)
{
    a = a - b;
    return a;
}

template <dse::util::enum_bit_ops_impl::bit_ops_enabled T>
T operator*(const T& a, dse::util::enum_bit_ops_impl::Bool b)
{
    return static_cast<T>(+a * b);
}

template <dse::util::enum_bit_ops_impl::bit_ops_enabled T>
T operator*(dse::util::enum_bit_ops_impl::Bool b, const T& a)
{
    return a * b;
}

#endif /* DSE_UTIL_ENUM_BITWISE_H_ */
