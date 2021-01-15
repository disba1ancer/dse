/*
 * unlock_guard.h
 *
 *  Created on: 22 . 2019 .
 *      Author: disba1ancer
 */

#ifndef UTIL_UNLOCK_GUARD_H
#define UTIL_UNLOCK_GUARD_H

namespace dse::util {

template <typename Mutex>
struct unlock_guard {
	typedef Mutex mutex_type;
	explicit unlock_guard(mutex_type& mtx) : m_mtx(mtx) {
		m_mtx.unlock();
	}
	~unlock_guard() {
		m_mtx.lock();
	}
	unlock_guard(const unlock_guard<mutex_type>&) = delete;
	unlock_guard<Mutex>& operator=(const unlock_guard<mutex_type>& mtx) = delete;
private:
	mutex_type& m_mtx;
};

} // namespace dse::util

#endif /* UTIL_UNLOCK_GUARD_H */
