/*
 * spinlock.h
 *
 *  Created on: 6 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_SPINLOCK_H_
#define THREADUTILS_SPINLOCK_H_

#include <atomic>

namespace dse {
namespace threadutils {

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

} // namespace threadutils
} // namespace dse

#endif /* THREADUTILS_SPINLOCK_H_ */
