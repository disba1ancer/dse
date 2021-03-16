/*
 * vector.h
 *
 *  Created on: 6 окт. 2018 г.
 *      Author: disba1ancer
 */

#ifndef VEC_H_
#define VEC_H_

#include <cstddef>
#include <type_traits>
#include <concepts>
#include <memory>

namespace dse::math {

template <typename value_t, std::size_t size>
struct vec;

namespace impl {

template <typename value_t>
concept no_cvref =
	!std::is_reference_v<value_t> &&
	!std::is_const_v<std::remove_reference_t<value_t>> &&
	!std::is_volatile_v<std::remove_reference_t<value_t>>;

template <typename value_t>
concept ref_or_val =
	std::is_lvalue_reference_v<value_t> ||
	no_cvref<value_t>;

template <typename>
constexpr bool is_vector = false;

template <typename type_t, std::size_t size>
constexpr bool is_vector<vec<type_t, size>> = true;

template <typename type_t>
concept scalar_type = !is_vector<std::remove_cv_t<type_t>>;

constexpr auto character_to_index(char ch) -> std::size_t {
	return (ch - 'x') & 3;
}

} // namespace impl

template <typename value_t, std::size_t size>
requires(
	!std::is_const_v<value_t> &&
	!std::is_volatile_v<value_t> &&
	!std::is_reference_v<value_t> &&
	size > 1
)
struct vec<value_t, size> {
	using value_type = value_t;
	using reference_type = value_t&;
	using const_reference_type = const value_t&;

	value_t elements[size];

	constexpr auto at_ptr(std::size_t index) -> value_t* { return elements + index; }
	constexpr auto at_ptr(std::size_t index) const -> const value_t* { return elements + index; }
	constexpr auto at(std::size_t index) -> reference_type { return elements[index]; }
	constexpr auto at(std::size_t index) const -> const_reference_type { return elements[index]; }
	constexpr auto operator[](std::size_t index) -> reference_type { return at(index); }
	constexpr auto operator[](std::size_t index) const -> const_reference_type { return at(index); }
private:
	template <typename dst_value_t, std::size_t... seq>
	requires(
		!std::is_reference_v<dst_value_t> && !std::is_const_v<dst_value_t> && !std::is_volatile_v<dst_value_t>
	)
	constexpr auto cast_impl(std::index_sequence<seq...>) const -> vec<dst_value_t, size> {
		return {static_cast<dst_value_t>(at(seq))...};
	}
public:
	template <typename dst_value_t>
	requires(
		!std::is_reference_v<dst_value_t> && !std::is_const_v<dst_value_t> && !std::is_volatile_v<dst_value_t>
	)
	constexpr operator vec<dst_value_t, size>() const {
		return cast_impl<dst_value_t>(std::make_index_sequence<size>{});
	}
private:
	template <std::size_t... seq>
	constexpr auto cast_impl(std::index_sequence<seq...>) const -> vec<const_reference_type, size> {
		return {at_ptr(seq)...};
	}
public:
	constexpr operator vec<const_reference_type, size>() const {
		return cast_impl(std::make_index_sequence<size>{});
	}
private:
	template <std::size_t... seq>
	constexpr auto cast_impl(std::index_sequence<seq...>) -> vec<reference_type, size> {
		return {at_ptr(seq)...};
	}
public:
	constexpr operator vec<reference_type, size>() {
		return cast_impl(std::make_index_sequence<size>{});
	}
private:
	template <std::size_t swizzle_length, std::size_t... seq>
	constexpr auto swizzle_impl(const char(&swizzle)[swizzle_length], std::index_sequence<seq...>) const
		-> vec<const_reference_type, swizzle_length - 1>
	{
		return {at_ptr(impl::character_to_index(swizzle[seq]))...};
	}
public:
	template <std::size_t swizzle_length>
	requires(2 < swizzle_length && swizzle_length <= 5)
	constexpr auto operator[](const char(&swizzle)[swizzle_length]) const
		-> vec<const_reference_type, swizzle_length - 1>
	{
		return swizzle_impl(swizzle, std::make_index_sequence<swizzle_length - 1>{});
	}
private:
	template <std::size_t swizzle_length, std::size_t... seq>
	constexpr auto swizzle_impl(const char(&swizzle)[swizzle_length], std::index_sequence<seq...>)
		-> vec<reference_type, swizzle_length - 1>
	{
		return {at_ptr(impl::character_to_index(swizzle[seq]))...};
	}
public:
	template <std::size_t swizzle_length>
	requires(2 < swizzle_length && swizzle_length <= 5)
	constexpr auto operator[](const char(&swizzle)[swizzle_length]) -> vec<reference_type, swizzle_length - 1> {
		return swizzle_impl(swizzle, std::make_index_sequence<swizzle_length - 1>{});
	}

	constexpr auto operator[](const char(&swizzle)[2]) const -> const_reference_type {
		return at(impl::character_to_index(swizzle[0]));
	}

	constexpr auto operator[](const char(&swizzle)[2]) -> reference_type {
		return at(impl::character_to_index(swizzle[0]));
	}
private:
	template <typename T>
	void assign_impl(const T& right) {
		for (std::size_t i = 0; i < size; ++i) {
			at(i) = right[i];
		}
	}
public:
//	auto operator=(const vec<value_type, size>& right) -> vec<value_type, size>&
//	{
//		assign_impl(right);
//		return *this;
//	}

	auto operator=(const vec<reference_type, size>& right) -> vec<value_type, size>&
	{
		assign_impl(right);
		return *this;
	}

	auto operator=(const vec<const_reference_type, size>& right) -> vec<value_type, size>&
	{
		assign_impl(right);
		return *this;
	}

	constexpr auto x() -> reference_type requires(size >= 1) { return at(0); }
	constexpr auto x() const -> const_reference_type requires(size >= 1) { return at(0); }
	constexpr auto y() -> reference_type requires(size >= 2) { return at(1); }
	constexpr auto y() const -> const_reference_type requires(size >= 2) { return at(1); }
	constexpr auto z() -> reference_type requires(size >= 3) { return at(2); }
	constexpr auto z() const -> const_reference_type requires(size >= 3) { return at(2); }
	constexpr auto w() -> reference_type requires(size >= 4) { return at(3); }
	constexpr auto w() const -> const_reference_type requires(size >= 4) { return at(3); }
};

template <impl::scalar_type value_t, std::size_t size>
struct vec<value_t&, size> {
	using value_type = std::remove_cv_t<value_t>;
	using reference_type = value_t&;
	using const_reference_type = const value_t&;

	value_t* elements[size];

	constexpr auto at_ptr(std::size_t index) const -> value_t* { return elements[index]; }
	constexpr auto at(std::size_t index) const -> reference_type { return *at_ptr(index); }
	constexpr auto operator[](std::size_t index) const -> reference_type { return at(index); }
private:
	template <typename dst_value_t, std::size_t... seq>
	requires(!std::is_reference_v<dst_value_t> && !std::is_const_v<dst_value_t> && !std::is_volatile_v<dst_value_t>)
	constexpr auto cast_impl(std::index_sequence<seq...>) const -> vec<dst_value_t, size> {
		return {static_cast<dst_value_t>(at(seq))...};
	}
public:
	template <typename dst_value_t>
	requires(!std::is_reference_v<dst_value_t> && !std::is_const_v<dst_value_t> && !std::is_volatile_v<dst_value_t>)
	constexpr operator vec<dst_value_t, size>() const {
		return cast_impl<dst_value_t>(std::make_index_sequence<size>{});
	}
private:
	template <std::size_t... seq>
	constexpr auto cast_impl_val(std::index_sequence<seq...>) const -> vec<value_type, size> {
		return {at(seq)...};
	}
public:
	constexpr operator vec<value_type, size>() const {
		return cast_impl_val(std::make_index_sequence<size>{});
	}
private:
	template <std::size_t... seq>
	constexpr auto cast_impl_ref(std::index_sequence<seq...>) const -> vec<const_reference_type, size> {
		return {at_ptr(seq)...};
	}

	constexpr operator vec<const_reference_type, size>() const {
		return cast_impl_ref(std::make_index_sequence<size>{});
	}
private:
	template <std::size_t swizzle_length, std::size_t... seq>
	auto swizzle_impl(const char(&swizzle)[swizzle_length], std::index_sequence<seq...>) const
		-> vec<reference_type, swizzle_length - 1>
	{
		return {at_ptr(impl::character_to_index(swizzle[seq]))...};
	}
public:
	template <std::size_t swizzle_length>
	requires(2 < swizzle_length && swizzle_length <= 5)
	auto operator[](const char(&swizzle)[swizzle_length]) const -> vec<reference_type, swizzle_length - 1> {
		return swizzle_impl(swizzle, std::make_index_sequence<swizzle_length - 1>{});
	}

	auto operator[](const char(&swizzle)[2]) const -> reference_type {
		return at(impl::character_to_index(swizzle[0]));
	}
private:
	template <typename T>
	void assign_impl(const T& right) const requires (!std::is_const_v<value_t>) {
		for (std::size_t i = 0; i < size; ++i) {
			at(i) = right[i];
		}
	}
public:
	auto operator=(const vec<value_type, size>& right) const -> const vec<reference_type, size>&
	requires (!std::is_const_v<value_t>)
	{
		assign_impl(right);
		return *this;
	}

	auto operator=(const vec<value_type&, size>& right) const -> const vec<reference_type, size>&
	requires (!std::is_const_v<value_t>)
	{
		assign_impl(right);
		return *this;
	}

	auto operator=(const vec<const value_type&, size>& right) const -> const vec<reference_type, size>&
	requires (!std::is_const_v<value_t>)
	{
		assign_impl(right);
		return *this;
	}

    constexpr auto x() const -> reference_type requires(size >= 1) { return at(0); }
    constexpr auto y() const -> reference_type requires(size >= 2) { return at(1); }
    constexpr auto z() const -> reference_type requires(size >= 3) { return at(2); }
    constexpr auto w() const -> reference_type requires(size >= 4) { return at(3); }
};

using uvec2 = vec<unsigned, 2>;
using uvec3 = vec<unsigned, 3>;
using uvec4 = vec<unsigned, 4>;

using ivec2 = vec<int, 2>;
using ivec3 = vec<int, 3>;
using ivec4 = vec<int, 4>;

using vec2 = vec<float, 2>;
using vec3 = vec<float, 3>;
using vec4 = vec<float, 4>;

/*template <typename type_t, std::size_t sizeA, std::size_t sizeB>
struct vec<vec<type_t, sizeA>, sizeB>;*/

} // namespace math::dse

#endif /* VEC_H_ */
