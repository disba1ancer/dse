/*
 * ThreadPoolwin32.h
 *
 *  Created on: 12 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_THREADPOOL_WIN32_H_
#define THREADUTILS_THREADPOOL_WIN32_H_

#include <list>
#include "os/win32.h"
#include <swal/error.h>
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
	typedef ThreadPool::Task Task;
	typedef ThreadPool::TaskId TaskId;
	typedef ThreadPool::TaskHandler TaskHandler;
	typedef ThreadPool::FinishHandler FinishHandler;
private:
	static constexpr auto WakeupMessage = WM_USER + 10;
public:
	ThreadPool_win32();
	~ThreadPool_win32();
	void schedule(Task& task);
	int join(ThreadType type);
	void stop(bool wait);
private:
	bool pop(Task*& task, ThreadType type);

	std::deque<Task*> scheduledTasks;
	spinlock mtx;
	std::mutex cvmtx;
	std::condition_variable condvar;
	bool running = false;
	int refs = 0;
	DWORD mainThread = 0;
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADUTILS_THREADPOOL_WIN32_H_ */
