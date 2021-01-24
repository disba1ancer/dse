/*
 * enum_bitwise.h
 *
 *  Created on: 12 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_ENUM_BITWISE_H_
#define UTIL_ENUM_BITWISE_H_

#include <algorithm>
#include <type_traits>

namespace dse {
namespace util {

template <typename T>
struct enable_enum_bitwise {
	static constexpr bool value = false;
};

namespace enum_bitwise {

template <typename Enum>
typename std::enable_if<dse::util::enable_enum_bitwise<Enum>::value, Enum>::type operator|(Enum enum1, Enum enum2) {
	return static_cast<Enum>(static_cast<typename std::underlying_type<Enum>::type>(enum1) | static_cast<typename std::underlying_type<Enum>::type>(enum2));
}

template <typename Enum>
typename std::enable_if<dse::util::enable_enum_bitwise<Enum>::value, Enum>::type operator&(Enum enum1, Enum enum2) {
	return static_cast<Enum>(static_cast<typename std::underlying_type<Enum>::type>(enum1) & static_cast<typename std::underlying_type<Enum>::type>(enum2));
}

template <typename Enum>
typename std::enable_if<dse::util::enable_enum_bitwise<Enum>::value, Enum&>::type operator|=(Enum& enum1, Enum enum2) {
	return enum1 = enum1 | enum2;
}

template <typename Enum>
typename std::enable_if<dse::util::enable_enum_bitwise<Enum>::value, Enum&>::type operator&=(Enum& enum1, Enum enum2) {
	return enum1 = enum1 & enum2;
}

}
}
}

#endif /* UTIL_ENUM_BITWISE_H_ */
