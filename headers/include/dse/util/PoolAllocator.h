/*
 * PoolAllocator.h
 *
 *  Created on: 18 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_POOLALLOCATOR_H_
#define UTIL_POOLALLOCATOR_H_

#include <vector>

namespace dse {
namespace util {

template <typename T>
class PoolAllocatorSingleton {
public:
	static PoolAllocatorSingleton& getInstance() {
		static PoolAllocatorSingleton alloc;
		return alloc;
	}
private:
	constexpr std::size_t blockSize = 8;
};

template <typename T>
class PoolAllocator {
public:
	typedef T value_type;

};

}
}

#endif /* UTIL_POOLALLOCATOR_H_ */
