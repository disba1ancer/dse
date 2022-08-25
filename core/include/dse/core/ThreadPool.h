/*
 * ThreadPool.h
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_THREADPOOL_H_
#define DSE_CORE_THREADPOOL_H_

#include <functional>
#include <thread>
#include <dse/util/pimpl.h>
#include <dse/util/functional.h>

namespace dse::core {

#ifdef _WIN32
typedef class ThreadPool_win32 ThreadPool_impl;
#endif

//enum class ThreadType {
//	Normal,
//	Main,
//	UI = Main
//};

enum class PoolCaps {
	OnlyWorkers = 0,
	IO = 1,
	UI = 2,
};

class IAsyncIO;

class ThreadPool : public util::pimpl<ThreadPool, ThreadPool_impl> {
public:
	typedef std::size_t TaskId;
	typedef std::function<void()> TaskHandler;
	typedef std::function<void()> FinishHandler;
	friend IAsyncIO;

	using Task = util::FunctionPtr<void()>;
//	class Task {
//	public:
//		friend ThreadPool_impl;
//		Task(TaskHandler&& taskHandler);
//		void then(FinishHandler&& fHandler);
//		void reset(TaskHandler&& taskHandler);
//	private:
//		TaskHandler taskHandler;
//		FinishHandler fHandler;
//	};
public:
	ThreadPool(unsigned int concurrency = std::thread::hardware_concurrency());
private:
	ThreadPool(const std::shared_ptr<ThreadPool_impl>& ptr);
public:
	ThreadPool(std::nullptr_t) noexcept;
	void Schedule(const Task& task);
	int Run(PoolCaps caps);
	[[deprecated]]
	void Join();
	void Stop();
	static const Task &GetCurrentTask();
	static ThreadPool GetCurrentPool();
};

} /* namespace dse::core */

#endif /* DSE_CORE_THREADPOOL_H_ */
