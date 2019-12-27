/*
 * mat.h
 *
 *  Created on: 24 февр. 2019 г.
 *      Author: Anton
 */

#ifndef MAT_H_
#define MAT_H_

#include "vec.h"

namespace dse {
namespace math {

template <typename type_t, unsigned sizeA, unsigned sizeB>
struct
alignas((sizeA * sizeB * sizeof(type_t) % 16) == 0 ? 16 : alignof(type_t))
vec<vec<type_t, sizeA>, sizeB> {
	typedef vec<type_t, sizeA> typev_t;
	typedef vec<typev_t, sizeB> self_t;

	vec<type_t, sizeA> elements[sizeB];

	self_t& operator +=(const self_t& other);
	self_t& operator -=(const self_t& other);
	self_t& operator -=(const type_t& other);
	self_t& operator *=(const type_t& other);
	self_t& operator /=(const type_t& other);
	self_t& negate();
	self_t operator +(const self_t& other) const;
	self_t operator -(const self_t& other) const;
	self_t operator -(const type_t& other) const;
	self_t operator *(const type_t& other) const;
	typev_t operator *(const typev_t& other) const;
	self_t operator /(const type_t& other) const;
	self_t operator -() const;
	typev_t& operator[](unsigned i);
	bool operator !=(const self_t& other) const;
	bool operator ==(const self_t& other) const;
	template <typename type_to_t>
	operator vec<vec<type_to_t, sizeA>, sizeB>() const;
};

template <typename type_t, unsigned sizeA, unsigned sizeB>
using mat = vec<vec<type_t, sizeA>, sizeB>;

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator +=(const self_t& other) -> self_t& {
	for (unsigned i = 0; i < sizeB; ++i) {
		elements[i] += other.elements[i];
	}
	return *this;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator -=(const self_t& other) -> self_t& {
	for (unsigned i = 0; i < sizeB; ++i) {
		elements[i] -= other.elements[i];
	}
	return *this;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator -=(const type_t& other) -> self_t& {
	for (unsigned i = 0; i < sizeB; ++i) {
		elements[i] -= other;
	}
	return *this;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator *=(const type_t& other) -> self_t& {
	for (unsigned i = 0; i < sizeB; ++i) {
		elements[i] *= other;
	}
	return *this;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator /=(const type_t& other) -> self_t& {
	for (unsigned i = 0; i < sizeB; ++i) {
		elements[i] /= other;
	}
	return *this;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator +(const self_t& other) const -> self_t {
	vec<vec<type_t, sizeA>, sizeB> result = *this;
	result += other;
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator -(const self_t& other) const -> self_t {
	vec<vec<type_t, sizeA>, sizeB> result = *this;
	result -= other;
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator -(const type_t& other) const -> self_t {
	vec<vec<type_t, sizeA>, sizeB> result = *this;
	result -= other;
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB, unsigned sizeC>
mat<type_t, sizeA, sizeC>
operator*(
	const mat<type_t, sizeA, sizeB>& m1,
	const mat<type_t, sizeB, sizeC>& m2
);

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator *(const typev_t& other) const -> typev_t {
	vec<typev_t, 1> oth;
	oth[0] = other;
	return (*this * oth)[0];
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator *(const type_t& other) const -> self_t {
	vec<vec<type_t, sizeA>, sizeB> result = *this;
	result *= other;
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator /(const type_t& other) const -> self_t {
	vec<vec<type_t, sizeA>, sizeB> result = *this;
	result /= other;
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::negate() -> self_t& {
	for (unsigned i = 0; i < sizeB; ++i) {
		elements[i] = -elements[i];
	}
	return *this;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator -() const -> self_t {
	auto result = *this;
	result.negate();
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
auto mat<type_t, sizeA, sizeB>::operator[](unsigned i) -> typev_t& {
	return elements[i];
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
bool mat<type_t, sizeA, sizeB>::operator !=(const self_t& other) const {
	for (unsigned i = 0; i < sizeB; ++i) {
		if (elements[i] != other.elements[i]) {
			return true;
		}
	}
	return false;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
bool mat<type_t, sizeA, sizeB>::operator ==(const self_t& other) const {
	return !(operator !=(other));
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
template <typename type_to_t>
mat<type_t, sizeA, sizeB>::operator mat<type_to_t, sizeA, sizeB>() const {
	vec<vec<type_t, sizeA>, sizeB> result;
	for (unsigned i = 0; i < sizeB; ++i) {
		result.elements[i] = this->elements[i];
	}
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB>
mat<type_t, sizeB, sizeA> transpose(const mat<type_t, sizeA, sizeB>& src) {
	mat<type_t, sizeB, sizeA> result;
	for (unsigned i = 0; i < sizeA; ++i) {
		for (unsigned j = 0; j < sizeA; ++j) {
			result.elements[j].elements[i] = src.elements[i].elements[j];
		}
	}
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB, unsigned sizeC>
mat<type_t, sizeA, sizeC>
operator*(
	const mat<type_t, sizeA, sizeB>& m1,
	const mat<type_t, sizeB, sizeC>& m2
) {
	mat<type_t, sizeA, sizeC> result;
	for (unsigned i = 0; i < sizeA; ++i) {
		for (unsigned j = 0; j < sizeC; ++j) {
			result.elements[i].elements[j] = 0;
			for (unsigned k = 0; k < sizeB; ++k) {
				result.elements[i].elements[j] +=
					m1.elements[k].elements[j] * m2.elements[i].elements[k];
			}
		}
	}
	return result;
}

template <typename type_t, unsigned size>
mat<type_t, size, size>&
operator*=(
	mat<type_t, size, size>& m1,
	const mat<type_t, size, size>& m2
) {
	for (unsigned i = 0; i < size; ++i) {
		vec<type_t, size> row;
		for (unsigned j = 0; j < size; ++j) {
			row.elements[j] = 0;
			for (unsigned k = 0; k < size; ++k) {
				row.elements[j] += m1.elements[k].elements[i] * m2.elements[j].elements[k];
			}
		}
		for (unsigned j = 0; j < size; ++j) {
			m1.elements[j].elements[i] = row.elements[j];
		}
	}
	return m1;
}

template <typename type_t, unsigned sizeA, unsigned sizeB, unsigned sizeC>
vec<type_t, sizeA>
operator*(
	const vec<type_t, sizeA>& v,
	const mat<type_t, sizeA, sizeB>& m
) {
	mat<type_t, sizeA, 1> v2;
	v2[0] = v;
	return (transpose(v2) * m)[0];
}

template <typename type_t, unsigned sizeA, unsigned sizeB, unsigned sizeC>
vec<type_t, sizeA>&
operator*=(
	vec<type_t, sizeA>& v,
	const mat<type_t, sizeA, sizeB>& m
) {
	v = v * m;
	return v;
}

/*extern template class vec<vec<float, 3>, 3>;
extern template class vec<vec<float, 4>, 4>;*/

typedef mat<float, 3, 3> mat3;
typedef mat<float, 4, 4> mat4;

/*extern template
mat<float, 3, 3> transpose(const mat<float, 3, 3>& src);

extern template
mat<float, 3, 3> operator *(const mat<float, 3, 3>&, const mat<float, 3, 3>&);

extern template
mat<float, 3, 3>& operator *=(mat<float, 3, 3>&, const mat<float, 3, 3>&);

extern template
mat<float, 4, 4> transpose(const mat<float, 4, 4>& src);

extern template
mat<float, 4, 4> operator *(const mat<float, 4, 4>&, const mat<float, 4, 4>&);

extern template
mat<float, 4, 4>& operator *=(mat<float, 4, 4>&, const mat<float, 4, 4>&);*/

} /* namespace math */
} /* namespace dse */

#endif /* MAT_H_ */
