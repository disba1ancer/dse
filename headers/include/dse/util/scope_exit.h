/*
 * FinalStep.h
 *
 *  Created on: 16 мая 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_UTIL_SCOPE_EXIT_H
#define DSE_UTIL_SCOPE_EXIT_H

#include <algorithm>

namespace dse::util {

template <typename T>
class scope_exit {
	T final;
public:
	scope_exit(const T& onFinal) : final(onFinal) {}
	scope_exit(T&& onFinal) : final(std::move(onFinal)) {}
	~scope_exit() noexcept(false) { final(); }
};

} // namespace dse::util

#endif /* DSE_UTIL_SCOPE_EXIT_H */
