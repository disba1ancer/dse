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
#include <vector>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <variant>
#include "TaskState.h"
#include "spinlock.h"
#include "util/pimpl.h"

namespace dse::threadutils {

#ifdef _WIN32
typedef class ThreadPool_win32 ThreadPool_impl;
#endif

enum class ThreadType {
	Normal,
	Main,
	UI = Main
};

enum class PoolCaps {
	OnlyWorkers = 0,
	IO = 1,
	UI = 2,
};

class IAsyncIO;

class ThreadPool : public util::pimpl<ThreadPool, ThreadPool_impl> {
public:
	typedef std::size_t TaskId;
	typedef std::function<TaskState()> TaskHandler;
	typedef std::function<void()> FinishHandler;
	friend IAsyncIO;

	class Task {
	public:
		friend ThreadPool_impl;
		Task(TaskHandler&& taskHandler);
		void then(FinishHandler&& fHandler);
		void reset(TaskHandler&& taskHandler);
	private:
		TaskHandler taskHandler;
		FinishHandler fHandler;
	};
public:
	ThreadPool(unsigned int concurrency = std::thread::hardware_concurrency());
private:
	ThreadPool(const std::shared_ptr<ThreadPool_impl>& ptr);
public:
	ThreadPool(std::nullptr_t) noexcept;
	void schedule(Task& task);
	int run(PoolCaps caps);
	[[deprecated]]
	void join();
	void stop();
	static Task* getCurrentTask();
	static ThreadPool getCurrentPool();
};

} /* namespace dse::threadutils */

#endif /* THREADPOOL_H_ */
