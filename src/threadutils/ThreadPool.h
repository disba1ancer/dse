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

namespace dse {
namespace threadutils {

#ifdef _WIN32
typedef class ThreadPool_win32 ThreadPool_impl;
#endif

enum class ThreadType {
	Normal,
	UI
};

class ThreadPoolTask {
public:
	TaskState state = TaskState::End;
	std::function<TaskState()> taskHandler;
	std::vector<std::function<void()>> finals;
};

class ThreadPool {
public:
	ThreadPool();
	~ThreadPool();
	std::size_t add(std::function<TaskState()>&& task, TaskState state = TaskState::Ready);
	void cancel(std::size_t taskId);
	void resume(std::size_t taskId);
	void then(std::size_t taskId, std::function<void()>&& then);
	int join(ThreadType type = ThreadType::Normal);
	void stop(bool wait = true);
private:
	ThreadPoolTask& allocTask(std::size_t& taskId);
	void schedule(std::size_t taskId);
	void remove(std::size_t taskId);

	std::vector<std::unique_ptr<ThreadPoolTask>> tasks;
	std::vector<std::size_t> freeTaskIds;
	std::deque<std::size_t> scheduledTasks;
	std::mutex mtx;
	std::condition_variable condvar;
	bool running = false;
	int refs;
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADPOOL_H_ */
