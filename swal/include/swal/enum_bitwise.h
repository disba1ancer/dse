/*
 * enum_bitwise.h
 *
 *  Created on: 12 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_ENUM_BITWISE_H
#define SWAL_ENUM_BITWISE_H

#include <algorithm>
#include <type_traits>

namespace swal {

template <typename T>
struct enable_enum_bitwise : std::false_type {};

template <typename T>
constexpr auto enable_enum_bitwise_v = enable_enum_bitwise<T>::value;

template <typename Enum>
Enum operator|(Enum enum1, Enum enum2) requires swal::enable_enum_bitwise_v<Enum> {
	return static_cast<Enum>(static_cast<typename std::underlying_type<Enum>::type>(enum1) | static_cast<typename std::underlying_type<Enum>::type>(enum2));
}

template <typename Enum>
Enum operator&(Enum enum1, Enum enum2) requires swal::enable_enum_bitwise_v<Enum> {
	return static_cast<Enum>(static_cast<typename std::underlying_type<Enum>::type>(enum1) & static_cast<typename std::underlying_type<Enum>::type>(enum2));
}

template <typename Enum>
Enum operator|=(Enum& enum1, Enum enum2) requires swal::enable_enum_bitwise_v<Enum> {
	return enum1 = enum1 | enum2;
}

template <typename Enum>
Enum operator&=(Enum& enum1, Enum enum2) requires swal::enable_enum_bitwise_v<Enum> {
	return enum1 = enum1 & enum2;
}

template <typename Enum>
Enum operator~(Enum& enum1) requires swal::enable_enum_bitwise_v<Enum> {
	return static_cast<Enum>(~static_cast<typename std::underlying_type<Enum>::type>(enum1));
}

}

#endif /* SWAL_ENUM_BITWISE_H */
