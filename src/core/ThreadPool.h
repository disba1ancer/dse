/*
 * ThreadPool.h
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <functional>
#include <thread>
#include <coroutine>
#include "util/pimpl.h"
#include "util/functional.h"

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

	using Task = util::function_view<void()>;
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
	void schedule(Task task);
	int run(PoolCaps caps);
	[[deprecated]]
	void join();
	void stop();
	static const Task &getCurrentTask();
	static ThreadPool getCurrentPool();
};

class ThreadPoolResume {
	ThreadPool& thrPool;
	std::coroutine_handle<> handle;
	ThreadPoolResume(ThreadPool& thrPool) : thrPool(thrPool) {}
	friend ThreadPoolResume operator co_await(ThreadPool& thrPool);
	void resumeTask() const {
		handle.resume();
	}
public:
	bool await_ready() {
		return false;
	}
	void await_suspend(std::coroutine_handle<> handle) {
		this->handle = handle;
		thrPool.schedule(util::from_method<&ThreadPoolResume::resumeTask>(*this));
	}
	void await_resume() {}
};

class pool_task_promise;

class pool_task {
public:
	using promise_type = pool_task_promise;
	friend class pool_task_promise;
private:
	std::coroutine_handle<promise_type> handle;
	pool_task(std::coroutine_handle<promise_type> handle) : handle(handle) {}
public:
	pool_task(const pool_task&) = default;
	pool_task(pool_task&&) = default;
	pool_task& operator=(const pool_task&) = default;
	pool_task& operator=(pool_task&&) = default;
	~pool_task() {
		handle.destroy();
	}
};

class pool_task_promise {
public:
	class initial_suspend_t {
		ThreadPool& pool;
	public:
		initial_suspend_t(ThreadPool& pool) : pool(pool) {}
		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<> handle) {
			pool.schedule({ handle.address(), [](void* h){
				auto handle = std::coroutine_handle<>::from_address(h);
				handle.resume();
			} });
		}
		void await_resume() {}
	};
private:
	ThreadPool& pool;
public:
	template<typename ... Args>
	pool_task_promise(ThreadPool& pool, Args&&...) : pool(pool) {}
	template<typename Cls, typename ... Args>
	pool_task_promise(Cls&&, ThreadPool& pool, Args&&...) : pool(pool) {}
	pool_task get_return_object() {
		return { std::coroutine_handle<pool_task_promise>::from_promise(*this) };
	}
	initial_suspend_t initial_suspend() {
		return { pool };
	}
	void return_void() {}
	void unhandled_exception() { std::terminate(); }
	std::suspend_always final_suspend() {
		return {};
	}
};

} /* namespace dse::core */

#endif /* THREADPOOL_H_ */
