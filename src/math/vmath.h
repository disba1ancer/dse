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

namespace dse {
namespace math {

template <typename T>
vec<T, 3> cross(const vec<T, 3>& a, const vec<T, 3>& b) {
	return {
		a[Y] * b[Z] - a[Z] * b[Y],
		a[Z] * b[X] - a[X] * b[Z],
		a[X] * b[Y] - a[Y] * b[X]
	};
}

template <typename T, unsigned size>
T dot(const vec<T, size>& a, const vec<T, size>& b) {
	T result = T();
	for (unsigned i = 0; i < size; ++i) {
		result += a[i] * b[i];
	}
	return result;
}

template <typename T, unsigned size>
vec<T, size> norm(const vec<T, size>& a) {
	T result = std::sqrt(dot(a, a));
	return a / result;
}

} // namespace math
} // namespace dse

#endif /* MATH_VMATH_H_ */
