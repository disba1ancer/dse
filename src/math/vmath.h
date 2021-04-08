/*
 * vmath.h
 *
 *  Created on: 6 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef MATH_VMATH_H_
#define MATH_VMATH_H_

#include "vec.h"
#include <cmath>

namespace dse::math {

namespace impl {

template <typename left_t, typename right_t>
concept same_storage_type = std::is_same_v<std::remove_cvref_t<left_t>, std::remove_cvref_t<right_t>>;

template <typename left_t, typename right_t, std::size_t size, typename Ret, std::size_t ... seq>
constexpr auto bin_op_impl(
    const vec<left_t, size>& left,
    const vec<right_t, size>& right,
    Ret(&func)(const std::remove_cvref_t<left_t>&, const std::remove_cvref_t<right_t>&),
    std::index_sequence<seq...>
) -> vec<Ret, size> {
    return {func(left[seq], right[seq])...};
}

template <typename left_t, typename right_t, std::size_t size, typename Ret, std::size_t ... seq>
constexpr auto bin_op_impl(
    const vec<left_t, size>& left,
    const right_t& right,
    Ret(&func)(const std::remove_cvref_t<left_t>&, const right_t&),
    std::index_sequence<seq...>
) -> vec<Ret, size> {
    return {func(left[seq], right)...};
}

template <typename left_t, typename right_t, std::size_t size, typename Ret, std::size_t ... seq>
constexpr auto bin_op_impl(
    const left_t& left,
    const vec<right_t, size>& right,
    Ret(&func)(const left_t&, const std::remove_cvref_t<right_t>&),
    std::index_sequence<seq...>
) -> vec<Ret, size> {
    return {func(left, right[seq])...};
}

template <typename right_t, std::size_t size, std::size_t ... seq>
constexpr auto negate_impl(
    const vec<right_t, size>& right,
    std::index_sequence<seq...>
) -> vec<std::remove_cvref_t<right_t>, size> {
    return {(-(right[seq]))...};
}

template <typename right_t, std::size_t size, std::size_t ... seq>
constexpr auto positive_impl(
    const vec<right_t, size>& right,
    std::index_sequence<seq...>
) -> vec<std::remove_cvref_t<right_t>, size> {
    return {(+(right[seq]))...};
}

} // namespace impl

#define GENERATE(op)\
template <typename left_t, typename right_t, std::size_t size>\
requires(\
	requires(left_t left, right_t right) {\
		left op right;\
	} &&\
	!impl::is_vector<std::remove_cvref_t<left_t>> &&\
	!impl::is_vector<std::remove_cvref_t<right_t>>\
)\
constexpr auto operator op(const vec<left_t, size>& left, const vec<right_t, size>& right) {\
	return impl::bin_op_impl(\
		left, right,\
		*[](const std::remove_cvref_t<left_t>& left, const std::remove_cvref_t<right_t>& right) { return left op right; },\
		std::make_index_sequence<size>{}\
	);\
}\
template <typename left_t, typename right_t, std::size_t size>\
requires(\
	requires(left_t left, right_t right) {\
		left op right;\
	} &&\
	!impl::is_vector<std::remove_cvref_t<left_t>> &&\
	!impl::is_vector<std::remove_cvref_t<right_t>>\
)\
constexpr auto operator op(const vec<left_t, size>& left, const right_t& right) {\
	return impl::bin_op_impl(\
		left, right,\
		*[](const std::remove_cvref_t<left_t>& left, const right_t& right) { return left op right; },\
		std::make_index_sequence<size>{}\
	);\
}\
template <typename left_t, typename right_t, std::size_t size>\
requires(\
	requires(left_t left, right_t right) {\
		left op right;\
	} &&\
	!impl::is_vector<std::remove_cvref_t<left_t>> &&\
	!impl::is_vector<std::remove_cvref_t<right_t>>\
)\
constexpr auto operator op(const left_t& left, const vec<right_t, size>& right) {\
	return impl::bin_op_impl(\
		left, right,\
		*[](const left_t& left, const std::remove_cvref_t<right_t>& right) { return left op right; },\
		std::make_index_sequence<size>{}\
	);\
}\
template <typename left_t, typename right_t, std::size_t size>\
auto operator op##=(vec<left_t, size>& left, const vec<right_t, size>& right) -> vec<left_t, size>&\
requires(\
	requires(left_t left, right_t right) {\
		left op##= right;\
	} &&\
	!impl::is_vector<std::remove_cvref_t<left_t>> &&\
	!impl::is_vector<std::remove_cvref_t<right_t>>\
)\
{\
	for (std::size_t i = 0; i < size; ++i) {\
		left[i] op##= right[i];\
	}\
	return left;\
}\
template <typename left_t, typename right_t, std::size_t size>\
auto operator op##=(vec<left_t, size>& left, const right_t& right) -> vec<left_t, size>&\
requires(\
	requires(left_t left, right_t right) {\
		left op##= right;\
	} &&\
	!impl::is_vector<std::remove_cvref_t<left_t>> &&\
	!impl::is_vector<std::remove_cvref_t<right_t>>\
)\
{\
	for (std::size_t i = 0; i < size; ++i) {\
		left[i] op##= right[i];\
	}\
	return left;\
}

GENERATE(+)
GENERATE(-)
GENERATE(*)
GENERATE(/)

#undef GENERATE

template <typename right_t, std::size_t size>
constexpr auto operator -(const vec<right_t, size>& right)
requires (
	requires(right_t right) { -right; }
) {
	return impl::negate_impl(right, std::make_index_sequence<size>{});
}

template <typename right_t, std::size_t size>
constexpr auto operator +(const vec<right_t, size>& right)
requires (
	requires(right_t right) { +right; }
) {
	return impl::negate_impl(right, std::make_index_sequence<size>{});
}

template <typename left_t, typename right_t, std::size_t size>
requires(impl::same_storage_type<left_t, right_t>)
constexpr bool operator !=(const vec<left_t, size>& left, const vec<right_t, size>& right) {
	for (unsigned i = 0; i < size; ++i) {
		if (left[i] != right[i]) {
			return true;
		}
	}
	return false;
}

template <typename left_t, typename right_t, std::size_t size>
requires(impl::same_storage_type<left_t, right_t>)
constexpr bool operator ==(const vec<left_t, size>& left, const vec<right_t, size>& other) {
	return !(left != other);
}

//template <typename type_t, std::size_t size>
//vec<type_t, size> operator *(const type_t& c, const vec<type_t, size>& v) {
//	vec<type_t, size> result = v;
//	result *= c;
//	return result;
//}

template <typename left_t, typename right_t>
requires(impl::same_storage_type<left_t, right_t>)
constexpr auto cross(const vec<left_t, 3>& a, const vec<right_t, 3>& b) -> vec<std::remove_cvref_t<left_t>, 3> {
	return a["yzx"] * b["zxy"] - a["zxy"] * b["yzx"];
}

template <typename left_t, typename right_t, std::size_t size>
requires(impl::same_storage_type<left_t, right_t>)
constexpr auto dot(const vec<left_t, size>& a, const vec<right_t, size>& b) {
	std::remove_cvref_t<left_t> result{};
	for (std::size_t i = 0; i < size; ++i) {
		result += a[i] * b[i];
	}
	return result;
}

template <typename T, std::size_t size>
constexpr vec<std::remove_cvref_t<T>, size> norm(const vec<T, size>& a) {
	return a / std::sqrt(dot(a, a));
}

} // namespace dse::math

#endif /* MATH_VMATH_H_ */
