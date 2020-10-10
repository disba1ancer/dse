/*
 * ThreadPoolwin32.h
 *
 *  Created on: 12 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_THREADPOOL_WIN32_H_
#define THREADUTILS_THREADPOOL_WIN32_H_

#include <list>
#include "ThreadPool.h"

namespace dse {
namespace threadutils {

class Task_win32 {
public:
private:
	std::function<void()> taskFunc;
};

class ThreadPool_win32 {
public:
	ThreadPool_win32();
	int join(ThreadType type = ThreadType::NORMAL);
	void spawnThreads(int count, ThreadType type = ThreadType::NORMAL);
	Task addTask(std::function<void()>&& taskFunc);
private:
	std::list<Task_win32> tasks;
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADUTILS_THREADPOOL_WIN32_H_ */
