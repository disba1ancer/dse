/*
 * qmath.h
 *
 *  Created on: 6 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef MATH_QMATH_H_
#define MATH_QMATH_H_

#include "vmath.h"
#include "mat.h"

namespace dse::math {

template <typename left_t, typename right_t>
requires(impl::same_storage_type<left_t, right_t>)
auto qmul(const vec<left_t, 4>& a, const vec<right_t, 4>& b) -> vec<std::remove_cvref_t<left_t>, 4> {
//	vec<std::remove_cvref_t<left_t>, 3> crs = a.w() * vec<T, 3>{b.x(), b.y(), b.z()} +
//			b.w() * vec<T, 3>{a.x(), a.y(), a.z()} +
//			cross(vec<T, 3>{a.x(), a.y(), a.z()}, vec<T, 3>{b.x(), b.y(), b.z()});
//	std::remove_cvref_t<left_t> dt = a.w() * b.w() - dot(vec<T, 3>{a.x(), a.y(), a.z()}, vec<T, 3>{b.x(), b.y(), b.z()});
//	return {
//		crs.x(),
//		crs.y(),
//		crs.z(),
//		dt
//	};
//	vec<std::remove_cvref_t<left_t>, 4> result;
//	result["xyz"] = a.w() * b["xyz"] + b.w() * a["xyz"] + cross(a["xyz"], b["xyz"]);
//	result.w() = a.w() * b.w() - dot(a["xyz"], b["xyz"]);
//	return result;
	return {(
		a["wwwx"] * b["xyzx"] +
		a["xyzy"] * b["wwwy"] +
		a["yzxz"] * b["zxyz"] +
		-a["zxyw"] * b["yzxw"]) *
		vec<std::remove_cvref_t<left_t>, 4>{1,1,1,-1}
	};
}

template <typename T>
auto qinv(const vec<T, 4>& a) -> vec<std::remove_cvref_t<T>, 4> {
	return {-a.x(), -a.y(), -a.z(), a.w()};
}

template <typename left_t, typename right_t>
requires(impl::same_storage_type<left_t, right_t>)
auto vecrotquat(const vec<left_t, 3>& _vec, const vec<right_t, 4>& quat) -> vec<std::remove_cvref_t<left_t>, 3> {
    return qmul(qmul(quat, vec<std::remove_cvref_t<left_t>, 4>{ _vec.x(), _vec.y(), _vec.z(), 0.f }), qinv(quat))["xyz"];
}

/*
 * vec4(a.w, a.z, -a.y, -a.x) vec4(-a.x, -a.y, -a.z, a.w)
 * vec4(-a.z, a.w, a.x, -a.y) vec4(-a.x, -a.y, -a.z, a.w)
 * vec4(a.y, -a.x, a.w, -a.z) vec4(-a.x, -a.y, -a.z, a.w)
 *
 * ( w, z,-y,-x) * (-x,-y,-z, w)
 * x*x - y*y - z*z + w*w
 * 2*(x*y + z*w)
 * 2*(x*z - y*w)
 * 0
 *
 * (-z, w, x,-y) * (-x,-y,-z, w)
 * 2*(x*y-z*w)
 * -x*x + y*y - z*z + w*w
 * 2*(y*z+x*w)
 * 0
 *
 * (y,-x, w,-z) vec4(-x,-y,-z, w)
 * 2*(x*z+y*w)
 * 2*(y*z-x*w)
 * -x*x-y*y+z*z+w*w
 * 0
 */

template <typename value_t>
auto matFromQuat(const vec<value_t, 4>& quat) -> mat<value_t, 3, 3> {
    auto xx = quat.x();
    auto yy = quat.y();
    auto zz = quat.z();
    auto ww = quat.w();
    auto xy = 2 * xx * yy;
    auto xz = 2 * xx * zz;
    auto xw = 2 * xx * ww;
    auto yz = 2 * yy * zz;
    auto yw = 2 * yy * ww;
    auto zw = 2 * zz * ww;
    xx *= xx;
    yy *= yy;
    zz *= zz;
    ww *= ww;
    return {
        xx - yy - zz + ww, xy + zw, xz - yw,
        xy - zw, -xx + yy - zz + ww, yz + xw,
        xz + yw, yz - xw, -xx - yy + zz + ww
    };
}

} // namespace dse::math

#endif /* MATH_QMATH_H_ */
