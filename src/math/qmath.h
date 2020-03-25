/*
 * qmath.h
 *
 *  Created on: 6 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef MATH_QMATH_H_
#define MATH_QMATH_H_

#include "vmath.h"

namespace dse {
namespace math {

template <typename T>
vec<T, 4> qmul(const vec<T, 4>& a, const vec<T, 4>& b) {
	vec<T, 3> crs = a[W] * vec<T, 3>{b[X], b[Y], b[Z]} +
			b[W] * vec<T, 3>{a[X], a[Y], a[Z]} +
			cross(vec<T, 3>{a[X], a[Y], a[Z]}, vec<T, 3>{b[X], b[Y], b[Z]});
	T dt = a[W] * b[W] - dot(vec<T, 3>{a[X], a[Y], a[Z]}, vec<T, 3>{b[X], b[Y], b[Z]});
	return {
		crs[X],
		crs[Y],
		crs[Z],
		dt
	};
}

template <typename T>
vec<T, 4> qinv(const vec<T, 4>& a) {
    return {-a[0], -a[1], -a[2], a[3]};
}

template <typename T>
vec<T, 3> vecrotquat(const vec<T, 3>& _vec, const vec<T, 4>& quat) {
	auto rslt = qmul(qmul(quat, { _vec[0], _vec[1], _vec[2], 0.f }), qinv(quat));
    return { rslt[0], rslt[1], rslt[2] };
}

} // namespace math
} // namespace dse

#endif /* MATH_QMATH_H_ */
