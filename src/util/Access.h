/*
 * Access.h
 *
 *  Created on: 19 мая 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_ACCESS_H_
#define UTIL_ACCESS_H_

#include <mutex>

namespace dse {
namespace util {

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

} // namespace util
} // namespace dse

#endif /* UTIL_ACCESS_H_ */
