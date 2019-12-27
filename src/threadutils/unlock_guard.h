/*
 * unlock_guard.h
 *
 *  Created on: 22 . 2019 .
 *      Author: Anton
 */

#ifndef UNLOCK_GUARD_H_
#define UNLOCK_GUARD_H_

namespace dse {
namespace threadutils {

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

}
}

#endif /* UNLOCK_GUARD_H_ */
