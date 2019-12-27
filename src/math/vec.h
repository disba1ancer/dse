/*
 * vector.h
 *
 *  Created on: 6 окт. 2018 г.
 *      Author: disba1ancer
 */

#ifndef VEC_H_
#define VEC_H_

namespace dse {
namespace math {

enum VecComponents {
	X,
	Y,
	Z,
	W,
};

template <typename type_t, unsigned size>
struct alignas((size * sizeof(type_t) % 16) == 0 ? 16 : alignof(type_t)) vec {

	type_t elements[size];

	vec<type_t, size>& operator +=(const vec<type_t, size>& other);
	vec<type_t, size>& operator -=(const vec<type_t, size>& other);
	vec<type_t, size>& operator -=(const type_t& other);
	vec<type_t, size>& operator *=(const vec<type_t, size>& other);
	vec<type_t, size>& operator *=(const type_t& other);
	vec<type_t, size>& operator /=(const vec<type_t, size>& other);
	vec<type_t, size>& operator /=(const type_t& other);
	vec<type_t, size>& negate();
	vec<type_t, size> operator +(const vec<type_t, size>& other) const;
	vec<type_t, size> operator -(const vec<type_t, size>& other) const;
	vec<type_t, size> operator -(const type_t& other) const;
	vec<type_t, size> operator *(const vec<type_t, size>& other) const;
	vec<type_t, size> operator *(const type_t& other) const;
	vec<type_t, size> operator /(const vec<type_t, size>& other) const;
	vec<type_t, size> operator /(const type_t& other) const;
	vec<type_t, size> operator -() const;
	type_t& operator[](unsigned i);
	bool operator !=(const vec<type_t, size>& other) const;
	bool operator ==(const vec<type_t, size>& other) const;
	template <typename type_to_t>
	operator vec<type_to_t, size>() const;
};

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator +=(const vec<type_t, size>& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] += other.elements[i];
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator -=(const vec<type_t, size>& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] -= other.elements[i];
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator -=(const type_t& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] -= other;
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator *=(const vec<type_t, size>& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] *= other.elements[i];
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator *=(const type_t& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] *= other;
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator /=(const vec<type_t, size>& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] /= other.elements[i];
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::operator /=(const type_t& other) {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] /= other;
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator +(const vec<type_t, size>& other) const {
	vec<type_t, size> result = *this;
	result += other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator -(const vec<type_t, size>& other) const {
	vec<type_t, size> result = *this;
	result -= other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator -(const type_t& other) const {
	vec<type_t, size> result = *this;
	result -= other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator *(const vec<type_t, size>& other) const {
	vec<type_t, size> result = *this;
	result *= other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator *(const type_t& other) const {
	vec<type_t, size> result = *this;
	result *= other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator /(const vec<type_t, size>& other) const {
	vec<type_t, size> result = *this;
	result /= other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator /(const type_t& other) const {
	vec<type_t, size> result = *this;
	result /= other;
	return result;
}

template <typename type_t, unsigned size>
vec<type_t, size>& vec<type_t, size>::negate() {
	for (unsigned i = 0; i < size; ++i) {
		elements[i] = -elements[i];
	}
	return *this;
}

template <typename type_t, unsigned size>
vec<type_t, size> vec<type_t, size>::operator -() const {
	auto result = *this;
	result.negate();
	return result;
}

template <typename type_t, unsigned size>
type_t& vec<type_t, size>::operator[](unsigned i) {
	return elements[i];
}

template <typename type_t, unsigned size>
bool vec<type_t, size>::operator !=(const vec<type_t, size>& other) const {
	for (unsigned i = 0; i < size; ++i) {
		if (elements[i] != other.elements[i]) {
			return true;
		}
	}
	return false;
}

template <typename type_t, unsigned size>
bool vec<type_t, size>::operator ==(const vec<type_t, size>& other) const {
	return !(operator !=(other));
}

template <typename type_t, unsigned size>
template <typename type_to_t>
vec<type_t, size>::operator vec<type_to_t, size>() const {
	vec<type_to_t, size> result;
	for (unsigned i = 0; i < size; ++i) {
		result.elements[i] = this->elements[i];
	}
	return result;
}


/*template <typename type_t, unsigned sizeA, unsigned sizeB>
vec<vec<type_t, sizeB>, sizeA> transpose(const vec<vec<type_t, sizeA>, sizeB>& src) {
	vec<vec<type_t, sizeB>, sizeA> result;
	for (unsigned i = 0; i < sizeA; ++i) {
		for (unsigned j = 0; j < sizeA; ++j) {
			result.elements[j].elements[i] = src.elements[i].elements[j];
		}
	}
	return result;
}

template <typename type_t, unsigned sizeA, unsigned sizeB, unsigned sizeC>
vec<vec<type_t, sizeA>, sizeC> operator*(const vec<vec<type_t, sizeA>, sizeB>& m1, const vec<vec<type_t, sizeB>, sizeC>& m2) {
	vec<vec<type_t, sizeA>, sizeC> result;
	for (unsigned i = 0; i < sizeA; ++i) {
		for (unsigned j = 0; j < sizeC; ++j) {
			result[i][j] = 0;
			for (unsigned k = 0; k < sizeB; ++k) {
				result.elements[i].elements[j] += m1.elements[i].elements[k] * m2.elements[k].elements[j];
			}
		}
	}
	return result;
}

template <typename type_t, unsigned size>
vec<vec<type_t, size>, size>& operator*=(vec<vec<type_t, size>, size>& m1, const vec<vec<type_t, size>, size>& m2) {
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
}*/

/*template <typename type_t, unsigned size>
template <unsigned offset>
vec<type_t, size>::PseudoField<offset>::operator type_t&() {
	return reinterpret_cast<vec<type_t, size>*>(this)->elements[offset];
}*/

/*extern template class vec<unsigned, 2>;
extern template class vec<unsigned, 3>;
extern template class vec<unsigned, 4>;

extern template class vec<int, 2>;
extern template class vec<int, 3>;
extern template class vec<int, 4>;

extern template class vec<float, 2>;
extern template class vec<float, 3>;
extern template class vec<float, 4>;*/

typedef vec<unsigned, 2> uvec2;
typedef vec<unsigned, 3> uvec3;
typedef vec<unsigned, 4> uvec4;

typedef vec<int, 2> ivec2;
typedef vec<int, 3> ivec3;
typedef vec<int, 4> ivec4;

typedef vec<float, 2> vec2;
typedef vec<float, 3> vec3;
typedef vec<float, 4> vec4;

template <typename type_t, unsigned sizeA, unsigned sizeB>
struct vec<vec<type_t, sizeA>, sizeB>;

} /* namespace math */
} /* namespace dse */

#endif /* VEC_H_ */
