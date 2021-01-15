/*
 * emptylock.h
 *
 *  Created on: 6 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_EMPTYLOCK_H_
#define THREADUTILS_EMPTYLOCK_H_

namespace dse {
namespace threadutils {

class emptylock {
public:
	void lock() {}
	bool try_lock() { return true; }
	void unlock() {}
};

} // namespace threadutils
} // namespace dse

#endif /* THREADUTILS_EMPTYLOCK_H_ */
