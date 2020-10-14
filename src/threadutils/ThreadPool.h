/*
 * ThreadPool.h
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <list>
#include <functional>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <memory>
#include "TaskState.h"

namespace dse {
namespace threadutils {

#ifdef _WIN32
typedef class ThreadPool_win32 ThreadPool_impl;
#endif

enum class ThreadType {
	NORMAL,
	UI
};

class ThreadPoolTask {
public:
private:
};

class ThreadPool {
public:
	ThreadPool();
	~ThreadPool();
	ThreadPoolTask& add(std::function<TaskState()> task);
	int join(ThreadType type);
private:
	typedef std::list<ThreadPoolTask> TaskList;
	TaskList tasks;
	std::deque<TaskList::iterator> tasksQueue;
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADPOOL_H_ */
