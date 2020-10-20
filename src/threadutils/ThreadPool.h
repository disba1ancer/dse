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
#include "TaskState.h"
#include "spinlock.h"

namespace dse {
namespace threadutils {

#ifdef _WIN32
typedef class ThreadPool_win32 ThreadPool_impl;
#endif

enum class ThreadType {
	Normal,
	Main,
	UI = Main
};

class ThreadPool {
public:
	typedef std::size_t TaskId;
	typedef std::function<TaskState()> TaskHandler;
	typedef std::function<void()> FinishHandler;

	class Task {
	public:
		friend ThreadPool_impl;
		Task(TaskHandler&& taskHandler);
		void cancel();
		void then(FinishHandler&& fHandler);
	private:
		TaskHandler taskHandler;
		FinishHandler fHandler;
		TaskState state = TaskState::Ready;
		spinlock mtx;
	};
public:
	ThreadPool();
	~ThreadPool();
	void schedule(Task& task);
	int join(ThreadType type = ThreadType::Normal);
	void stop(bool wait = true);
private:
	std::unique_ptr<ThreadPool_impl> impl;
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADPOOL_H_ */
