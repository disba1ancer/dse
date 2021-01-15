/*
 * ThreadPoolwin32.h
 *
 *  Created on: 12 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_THREADPOOL_WIN32_H_
#define THREADUTILS_THREADPOOL_WIN32_H_

#include <swal/handle.h>
#include <deque>
#include <vector>
#include "ThreadPool.h"
#include "util/spinlock.h"

namespace dse::core {

class IAsyncIO : public OVERLAPPED {
public:
	virtual void complete(DWORD transfered, DWORD error) = 0;
protected:
	static std::shared_ptr<ThreadPool_win32> getImplFromPool(ThreadPool& pool);
};

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
		ThreadPool_win32* currentPool = nullptr;
		Task currentTask = nullptr;
		struct slmover : util::spinlock {
			slmover() noexcept = default;
			slmover(const slmover&) = delete;
			slmover(slmover&&) noexcept {}
			slmover& operator=(const slmover&) = delete;
			slmover& operator=(slmover&&) noexcept {}
		} mtx;
		std::deque<Task> taskQueue;
		swal::CompletionStatusResult ioCompletion = {};
		bool isMain;
	} *thrDataPtr;
//	std::deque<Task*> scheduledTasks;
//	std::mutex cvmtx;
//	std::condition_variable condvar;
	std::atomic_bool running = false;
	std::atomic_bool slpThrds = false;
	std::atomic_int refs = 0;
	PoolCaps caps;
	DWORD mainThread = 0;
	std::vector<ThreadData> threadsData;
	swal::IOCompletionPort iocp;
public:
	ThreadPool_win32(unsigned int concurrency);
	~ThreadPool_win32();
	void schedule(Task task);
	int run(PoolCaps caps);
	void join();
	void stop();
	static const Task& getCurrentTask();
	static std::shared_ptr<ThreadPool_win32> getCurrentPool();
	void iocpAttach(swal::Handle& handle);
private:
	void join(bool isMain);
	void thrEntry(std::size_t dataIdx);
	void handleMessages();
	void trySteal(ThreadData& thrData);
	Task pop(ThreadData& thrData);
	void wakeupThreads();
	void handleIO(ThreadData& thrData);
	void waitForWork(ThreadData& thrData);
	std::size_t getTasksCount(ThreadData &thrData);
	void handleSystem(ThreadData& thrData);
};

} /* namespace dse::core */

#endif /* THREADUTILS_THREADPOOL_WIN32_H_ */
