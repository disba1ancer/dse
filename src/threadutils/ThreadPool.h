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
	NORMAL,
	UI
};

class TaskInternal : std::enable_shared_from_this<TaskInternal> {
public:
	TaskInternal(std::function<void()>&& func);
	void then(std::function<void()>&& taskFunc);
	void run();
private:
	std::function<void()> taskFunc;
	std::list<std::function<void()>> continuations;
};

class Task {
public:
	Task(std::shared_ptr<TaskInternal> intern);
	void then(std::function<void()>&& taskFunc);
private:
	std::shared_ptr<TaskInternal> internal;
};

class ThreadPool {
public:
	ThreadPool();
	~ThreadPool();
	int join(ThreadType type = ThreadType::NORMAL);
	void spawnThreads(int count, ThreadType type = ThreadType::NORMAL);
	Task addTask(std::function<void()>&& taskFunc);
	void stop();
	//void addTask(Task& task);
private:
	std::mutex mtx;
	std::condition_variable condVar;
	int threadCount = 0;
	bool isStop = false;
	std::deque<std::shared_ptr<TaskInternal>> tasksQueue;
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADPOOL_H_ */
