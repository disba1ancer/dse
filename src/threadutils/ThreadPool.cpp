/*
 * ThreadPool.cpp
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "ThreadPool.h"
#include "unlock_guard.h"

#ifdef _WIN32
#include "ThreadPool_win32.h"
#endif

namespace dse {
namespace threadutils {

ThreadPool::ThreadPool() {
}

ThreadPool::~ThreadPool() {

}

} /* namespace threadutils */
} /* namespace dse */
