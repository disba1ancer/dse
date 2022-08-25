/*
 * spinlock.h
 *
 *  Created on: 6 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_UTIL_SPINLOCK_H
#define DSE_UTIL_SPINLOCK_H

#include <atomic>

namespace dse::util {

class spinlock {
	std::atomic_flag isLock;
public:
	void lock() {
		while (isLock.test_and_set(std::memory_order_acquire));
	}
	bool try_lock() {
		return !(isLock.test_and_set(std::memory_order_acquire));
	}
	void unlock() {
		isLock.clear(std::memory_order_release);
	}
};

} // namespace dse::util

#endif /* DSE_UTIL_SPINLOCK_H */
