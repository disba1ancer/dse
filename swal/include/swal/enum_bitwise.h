/*
 * enum_bitwise.h
 *
 *  Created on: 12 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_ENUM_BITWISE_H_
#define SWAL_ENUM_BITWISE_H_

#include <algorithm>
#include <type_traits>

namespace swal {

template <typename T>
struct enable_enum_bitwise {
	static constexpr bool value = false;
};

template <typename Enum>
typename std::enable_if<swal::enable_enum_bitwise<Enum>::value, Enum>::type operator|(Enum enum1, Enum enum2) {
	return static_cast<Enum>(static_cast<typename std::underlying_type<Enum>::type>(enum1) | static_cast<typename std::underlying_type<Enum>::type>(enum2));
}

template <typename Enum>
typename std::enable_if<swal::enable_enum_bitwise<Enum>::value, Enum>::type operator&(Enum enum1, Enum enum2) {
	return static_cast<Enum>(static_cast<typename std::underlying_type<Enum>::type>(enum1) & static_cast<typename std::underlying_type<Enum>::type>(enum2));
}

template <typename Enum>
typename std::enable_if<swal::enable_enum_bitwise<Enum>::value, Enum&>::type operator|=(Enum& enum1, Enum enum2) {
	return enum1 = enum1 | enum2;
}

template <typename Enum>
typename std::enable_if<swal::enable_enum_bitwise<Enum>::value, Enum&>::type operator&=(Enum& enum1, Enum enum2) {
	return enum1 = enum1 & enum2;
}

template <typename Enum>
typename std::enable_if<swal::enable_enum_bitwise<Enum>::value, Enum&>::type operator~(Enum& enum1) {
	return static_cast<Enum>(~static_cast<typename std::underlying_type<Enum>::type>(enum1));
}

}

#endif /* SWAL_ENUM_BITWISE_H_ */
