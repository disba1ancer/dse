/*
 * emptylock.h
 *
 *  Created on: 6 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_UTIL_EMPTYLOCK_H_
#define DSE_UTIL_EMPTYLOCK_H_

namespace dse::util {

class emptylock {
public:
	void lock() {}
	bool try_lock() { return true; }
	void unlock() {}
};

} // namespace dse::util

#endif /* DSE_UTIL_EMPTYLOCK_H_ */
