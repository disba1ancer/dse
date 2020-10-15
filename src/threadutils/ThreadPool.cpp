/*
 * ThreadPool.cpp
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "ThreadPool.h"
#include "unlock_guard.h"
#include "util/Access.h"

#ifdef _WIN32
#include "ThreadPool_win32.h"
#endif

namespace dse {
namespace threadutils {

ThreadPool::ThreadPool() {
}

ThreadPool::~ThreadPool() {

}

std::size_t ThreadPool::add(std::function<TaskState()>&& taskHandler, TaskState state) {
	std::scoped_lock lcks(mtx);
	std::size_t taskId;
	auto& task = allocTask(taskId);
	task.taskHandler = std::move(taskHandler);
	task.state = state;
	if (state == TaskState::Ready) schedule(taskId);
	return taskId;
}

void ThreadPool::cancel(std::size_t taskId) {
	std::scoped_lock lcks(mtx);
	auto& task = *(tasks[taskId]);
	task.state = TaskState::Canceled;
}
void ThreadPool::resume(std::size_t taskId) {
	std::scoped_lock lcks(mtx);
	auto& task = *(tasks[taskId]);
	switch (task.state) {
	case TaskState::Await:
		task.state = TaskState::Ready;
		[[fallthrough]];
	case TaskState::Canceled:
		schedule(taskId);
		break;
	default:
		throw std::runtime_error("Task in unexpected state");
	}
}

void ThreadPool::then(std::size_t taskId, std::function<void()>&& then) {
	std::scoped_lock lcks(mtx);
	tasks[taskId]->finals.emplace_back(std::move(then));
}

int ThreadPool::join(ThreadType type) {
	std::unique_lock lck(mtx);
	running = true;
	++refs;
	while (running) {
		auto scheduledCount = scheduledTasks.size();
		for (std::size_t i = 0; i < scheduledCount; ++i) {
			auto taskId = scheduledTasks.front();
			scheduledTasks.pop_front();
			auto& task = *(tasks[taskId]);
			auto& state = task.state;
			if (state != TaskState::Canceled) {
				unlock_guard ulck(lck);
				state = task.taskHandler();
			}
			switch (state) {
			case TaskState::Ready:
				schedule(taskId);
				break;
			case TaskState::End:
				for (auto& final : task.finals) {
					final();
				}
				[[fallthrough]];
			case TaskState::Canceled:
				remove(taskId);
				break;
			case TaskState::Await:
				break;
			}
		}
		if (scheduledTasks.size() == 0) {
			condvar.wait(lck);
		}
	}
	--refs;
}

void ThreadPool::stop(bool wait) {
	{
		std::scoped_lock lck(mtx);
		running = false;
		condvar.notify_all();
	}
	while(wait && bool(util::access(mtx, refs))) {}
}

ThreadPoolTask& ThreadPool::allocTask(std::size_t& taskId) {
	if (freeTaskIds.size()) {
		taskId = freeTaskIds.back();
		freeTaskIds.pop_back();
		auto& task = tasks[taskId];
		task = std::make_unique<ThreadPoolTask>();
		return *task;
	} else {
		tasks.emplace_back();
		taskId = tasks.size() - 1;
		auto& task = tasks[taskId];
		task = std::make_unique<ThreadPoolTask>();
		return *task;
	}
}

void ThreadPool::schedule(std::size_t taskId) {
	scheduledTasks.push_back(taskId);
	condvar.notify_one();
}

void ThreadPool::remove(std::size_t taskId) {
	tasks[taskId].reset();
	freeTaskIds.emplace_back(taskId);
}

} /* namespace threadutils */
} /* namespace dse */
