/*
 * Access.h
 *
 *  Created on: 19 мая 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_ACCESS_H
#define UTIL_ACCESS_H

#include <mutex>

namespace dse::util {

template <typename lockable, typename value>
value access(lockable& l, const value& val) {
	std::scoped_lock lck(l);
	return val;
}
/*class Access {
	lockable& l;
	const value& val;
public:
	Access(lockable& l, const value& val) : l(l), val(val) {}
	operator value() { std::lock_guard lck(l); return val; }
};*/

} // namespace dse::util

#endif /* UTIL_ACCESS_H */
