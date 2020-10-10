/*
 * ThreadPool.cpp
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "ThreadPool.h"
#include "unlock_guard.h"

#ifdef _WIN32
#include "ThreadPool_win32.h"
#endif

namespace dse {
namespace threadutils {

ThreadPool::ThreadPool() {
}

int ThreadPool::join(ThreadType type) {
	std::unique_lock lock(mtx);
	++threadCount;
	//currentThread = this;
	while (!isStop) {
		if (!tasksQueue.empty()){
			auto task = tasksQueue.front();
			tasksQueue.pop_front();
			//TaskState state;
			{
				unlock_guard unlock(mtx);
				task->run();
			}
			/*switch (state) {
			case TaskState::YIELD:
				if (!isExit) queuedTasks.emplace_back(std::move(task));
				break;
			case TaskState::END:
				tasks.erase(task.taskid);
				break;
			case TaskState::AWAIT:
				break;
			}*/
		} else {
			condVar.wait(lock);
		}
	}
	tasksQueue.clear();
	//tasks.clear();
	if (!--threadCount) {
		isStop = false;
	}
	return 0;
}

ThreadPool::~ThreadPool() {
	std::unique_lock lock(mtx);
}

Task ThreadPool::addTask(std::function<void()>&& taskFunc) {
	std::lock_guard lock(mtx);
	auto task = std::make_shared<TaskInternal>(std::move(taskFunc));
	tasksQueue.push_back(task);
	return Task(task);
}

TaskInternal::TaskInternal(std::function<void()>&& function) : taskFunc(std::move(function)) {
}

Task::Task(std::shared_ptr<TaskInternal> intern) : internal(intern) {
}

void TaskInternal::then(std::function<void()>&& taskFunc) {
	continuations.emplace_back(std::move(taskFunc));
}

void Task::then(std::function<void()>&& taskFunc) {
	internal->then(std::move(taskFunc));
}

void ThreadPool::stop() {
	isStop = true;
}

void TaskInternal::run() {
	taskFunc();
	for (auto continuation : continuations) {
		continuation();
	}
}

} /* namespace threadutils */
} /* namespace dse */
