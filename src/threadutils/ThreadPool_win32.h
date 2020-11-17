/*
 * ThreadPoolwin32.h
 *
 *  Created on: 12 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_THREADPOOL_WIN32_H_
#define THREADUTILS_THREADPOOL_WIN32_H_

#include <deque>
#include <vector>
#include "os/win32.h"
#include <swal/error.h>
#include "ThreadPool.h"

namespace dse::threadutils {

class ThreadPool_win32 : public std::enable_shared_from_this<ThreadPool_win32> {
public:
	typedef ThreadPool::Task Task;
	typedef ThreadPool::TaskId TaskId;
	typedef ThreadPool::TaskHandler TaskHandler;
	typedef ThreadPool::FinishHandler FinishHandler;
private:
	static constexpr auto WakeupMessage = WM_USER + 0x10;
	//static thread_local std::weak_ptr<ThreadPool_win32> currentPool;
	//static thread_local Task* currentTask;
	static thread_local struct ThreadData {
		std::thread thr;
		ThreadPool_win32* currentPool;
		Task* currentTask;
		spinlock mtx;
		std::deque<Task*> taskQueue;
	} *thrDataPtr;

public:
	ThreadPool_win32(unsigned int concurrency = std::thread::hardware_concurrency());
	~ThreadPool_win32();
	void schedule(Task& task);
	int run(PoolCaps caps);
	void join();
	void stop();
	static Task* getCurrentTask();
	static std::weak_ptr<ThreadPool_win32> getCurrentPool();
private:
	void join(bool isMain);
	bool pop(ThreadData& thrData, bool isMain);
	void thrEntry(std::size_t dataIdx);

	//std::deque<Task*> scheduledTasks;
	std::mutex cvmtx;
	std::condition_variable condvar;
	std::atomic_bool running = false;
	std::atomic_int refs = 0;
	PoolCaps caps;
	DWORD mainThread = 0;
	std::vector<ThreadData> threadsData;
};

} /* namespace dse::threadutils */

#endif /* THREADUTILS_THREADPOOL_WIN32_H_ */
