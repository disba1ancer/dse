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
#include "detail/impexp.h"

namespace dse::core {

#ifdef _WIN32
typedef class ThreadPool_win32 ThreadPool_impl;
#endif

enum class PoolCaps {
	OnlyWorkers = 0,
	IO = 1,
	UI = 2,
};

class IAsyncIO;

class API_DSE_CORE ThreadPool {
public:
	typedef std::size_t TaskId;
	typedef std::function<void()> TaskHandler;
	typedef std::function<void()> FinishHandler;
    friend ThreadPool_impl;
	friend IAsyncIO;

	using Task = util::function_ptr<void()>;
public:
	ThreadPool(unsigned int concurrency = std::thread::hardware_concurrency());
    ~ThreadPool();
public:
	void Schedule(const Task& task);
	int Run(PoolCaps caps);
	[[deprecated]]
	void Join();
	void Stop();
	static const Task &GetCurrentTask();
	static ThreadPool* GetCurrentPool();
private:
    util::impl_ptr<ThreadPool_impl> impl;
};

} /* namespace dse::core */

#endif /* DSE_CORE_THREADPOOL_H_ */
