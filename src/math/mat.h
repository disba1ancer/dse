/*
 * mat.h
 *
 *  Created on: 24 февр. 2019 г.
 *      Author: disba1ancer
 */

#ifndef MAT_H_
#define MAT_H_

#include "vmath.h"

namespace dse::math {

template <typename type_t, std::size_t cols, std::size_t rows>
using mat = vec<vec<type_t, rows>, cols>;

namespace impl {

} // namespace impl

#define GENERATE(op)\
template <typename left_t, typename right_t, std::size_t size>\
requires(\
	requires(left_t left, right_t right) {\
		left op right;\
	} &&\
	impl::is_vector<std::remove_cv_t<left_t>> &&\
	impl::is_vector<std::remove_cv_t<right_t>>\
)\
constexpr auto operator op(const vec<left_t, size>& left, const vec<right_t, size>& right) {\
	return impl::bin_op_impl(\
		left, right,\
		*[](const left_t& left, const right_t& right) { return left op right; },\
		std::make_index_sequence<size>{}\
	);\
}\
template <typename left_t, typename right_t, std::size_t size>\
requires(\
	requires(left_t left, right_t right) {\
		left op right;\
	} &&\
	impl::is_vector<std::remove_cv_t<left_t>> &&\
	impl::is_vector<std::remove_cv_t<right_t>>\
)\
constexpr auto operator op(const vec<left_t, size>& left, const right_t& right) {\
	return impl::bin_op_impl(\
		left, right,\
		*[](const left_t& left, const right_t& right) { return left op right; },\
		std::make_index_sequence<size>{}\
	);\
}\
template <typename left_t, typename right_t, std::size_t size>\
requires(\
	requires(left_t left, right_t right) {\
		left op right;\
	} &&\
	impl::is_vector<std::remove_cv_t<left_t>> &&\
	impl::is_vector<std::remove_cv_t<right_t>>\
)\
constexpr auto operator op(const left_t& left, const vec<right_t, size>& right) {\
	return impl::bin_op_impl(\
		left, right,\
		*[](const left_t& left, const right_t& right) { return left op right; },\
		std::make_index_sequence<size>{}\
	);\
}\
template <typename left_t, typename right_t, std::size_t size>\
auto operator op##=(vec<left_t, size>& left, const vec<right_t, size>& right) -> vec<left_t, size>&\
requires(\
	requires(left_t left, right_t right) {\
		left op##= right;\
	} &&\
	impl::is_vector<std::remove_cv_t<left_t>> &&\
	impl::is_vector<std::remove_cv_t<right_t>>\
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
	impl::is_vector<std::remove_cv_t<left_t>> &&\
	impl::is_vector<std::remove_cv_t<right_t>>\
)\
{\
	for (std::size_t i = 0; i < size; ++i) {\
		left[i] op##= right[i];\
	}\
	return left;\
}

GENERATE(+)
GENERATE(-)

#undef GENERATE

template <typename left_t, typename right_t, std::size_t rows, std::size_t shared, std::size_t cols>
auto operator*(const mat<left_t, shared, rows>& left, const mat<right_t, cols, shared>& right) {
    using ret_t = decltype(left_t{} * right_t{});
    mat<ret_t, shared, rows> result{};
    for (std::size_t col = 0; col < cols; ++col) {
        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t i = 0; i < shared; ++i) {
                result[col][row] += left[i][row] * right[col][i];
            }
        }
    }
    return result;
}

template <typename left_t, typename right_t, std::size_t cols, std::size_t rows>
mat<left_t, cols, rows>& operator*=(mat<left_t, cols, rows>& left, const mat<right_t, cols, cols>& right) {
    left = left * right;
    return left;
}

template <typename right_t, std::size_t cols, std::size_t rows>
auto transpose(const mat<right_t, cols, rows>& right) {
    mat<std::remove_cvref_t<right_t>, rows, cols> result{};
    for (std::size_t i = 0; i < cols; ++i) {
        for (std::size_t j = 0; j < rows; ++j) {
            result[j][i] = right[i][j];
        }
    }
    return result;
}

typedef mat<float, 2, 2> mat2;
typedef mat<float, 2, 3> mat2x3;
typedef mat<float, 2, 4> mat2x4;
typedef mat<float, 3, 2> mat3x2;
typedef mat<float, 3, 3> mat3;
typedef mat<float, 3, 4> mat3x4;
typedef mat<float, 4, 2> mat4x2;
typedef mat<float, 4, 3> mat4x3;
typedef mat<float, 4, 4> mat4;

} /* namespace dse::math */

#endif /* MAT_H_ */
