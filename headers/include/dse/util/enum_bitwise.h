/*
 * enum_bitwise.h
 *
 *  Created on: 12 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_UTIL_ENUM_BITWISE_H_
#define DSE_UTIL_ENUM_BITWISE_H_

#include <algorithm>
#include <type_traits>

namespace dse {
namespace util {

template <typename T>
struct enable_enum_bitwise {
	static constexpr bool value = false;
};

template <typename T>
constexpr bool enable_enum_bitwise_v = enable_enum_bitwise<T>::value;

}
}

template <typename Enum>
Enum operator|(Enum enum1, Enum enum2) requires (dse::util::enable_enum_bitwise_v<Enum>) {
	using und_t = typename std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<und_t>(enum1) | static_cast<und_t>(enum2));
}

template <typename Enum>
Enum operator&(Enum enum1, Enum enum2) requires (dse::util::enable_enum_bitwise_v<Enum>) {
	using und_t = typename std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<und_t>(enum1) & static_cast<und_t>(enum2));
}

template <typename Enum>
Enum& operator|=(Enum& enum1, Enum enum2) requires (dse::util::enable_enum_bitwise_v<Enum>) {
	return enum1 = enum1 | enum2;
}

template <typename Enum>
Enum operator&=(Enum& enum1, Enum enum2) requires (dse::util::enable_enum_bitwise_v<Enum>) {
	return enum1 = enum1 & enum2;
}

#endif /* DSE_UTIL_ENUM_BITWISE_H_ */
