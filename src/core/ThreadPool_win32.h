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
	virtual void Complete(DWORD transfered, DWORD error) = 0;
protected:
	static std::shared_ptr<ThreadPool_win32> GetImplFromPool(ThreadPool& pool);
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
			slmover& operator=(slmover&&) noexcept { return *this; }
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
	void Schedule(const Task& task);
	int Run(PoolCaps caps);
	void Join();
	void Stop();
	static const Task& GetCurrentTask();
	static std::shared_ptr<ThreadPool_win32> GetCurrentPool();
	void IOCPAttach(swal::Handle& handle);
private:
	void Join(bool isMain);
	void ThrEntry(std::size_t dataIdx);
	void HandleMessages();
	void TrySteal(ThreadData& thrData);
	Task Pop(ThreadData& thrData);
	void WakeupThreads();
	void HandleIO(ThreadData& thrData);
	void WaitForWork(ThreadData& thrData);
	std::size_t GetTasksCount(ThreadData &thrData);
	void HandleSystem(ThreadData& thrData);
};

} /* namespace dse::core */

#endif /* THREADUTILS_THREADPOOL_WIN32_H_ */
