/*
 * ExecutionThread.cpp
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "ExecutionThread.h"
#include "unlock_guard.h"

namespace dse {
namespace threadutils {

void ExecutionThread::threadEntry() {
	std::unique_lock lock(mtx);
	currentThread = this;
	while (m_isRun && !(isExit && queuedTasks.empty())) {
		if (!queuedTasks.empty()){
			auto task = queuedTasks.front();
			queuedTasks.pop_front();
			TaskState state;
			{
				unlock_guard unlock(mtx);
				state = (*task.taskid)();
			}
			switch (state) {
			case TaskState::YIELD:
				if (!isExit) queuedTasks.emplace_back(std::move(task));
				break;
			case TaskState::END:
				tasks.erase(task.taskid);
				break;
			case TaskState::AWAIT:
				break;
			}
		} else {
			cond_var.wait(lock);
		}
	}
	queuedTasks.clear();
	tasks.clear();
	isExit = false;
	m_isRun = false;
}

void ExecutionThread::run() {
	m_isRun = true;
	result = -1;
	thread = std::thread(&ExecutionThread::threadEntry, this);
}

int ExecutionThread::runOnCurent() {
	m_isRun = true;
	result = -1;
	threadEntry();
	return result;
}

void ExecutionThread::join() {
	if (thread.joinable()) thread.join();
}

void ExecutionThread::terminate() {
	std::lock_guard lock(mtx);
	m_isRun = false;
	cond_var.notify_one();
}

void ExecutionThread::exit(int r) {
	std::lock_guard lock(mtx);
	result = r;
	isExit = true;
	cond_var.notify_one();
}

 auto ExecutionThread::addTask(Task &&task) -> std::optional<TaskID> {
	std::lock_guard lock(mtx);
	if (!isExit) {
		TaskID id{ tasks.emplace(tasks.end(), std::move(task)) };
		queuedTasks.emplace_back(id);
		cond_var.notify_one();
		return id;
	}
	return {};
}

bool ExecutionThread::isRun() {
	std::lock_guard lock(mtx);
	return m_isRun;
}

bool ExecutionThread::getResult() {
	return result;
}

thread_local ExecutionThread* ExecutionThread::currentThread = nullptr;

ExecutionThread& ExecutionThread::getCurrentThread() {
	return *currentThread;
}

void ExecutionThread::yieldTasks() {
	std::unique_lock lock(mtx);
	if (currentThread == this) {
		auto taskCount = queuedTasks.size();
		for (decltype(taskCount) i = 0; m_isRun && i < taskCount && !queuedTasks.empty(); ++i) {
			auto task = queuedTasks.front();
			queuedTasks.pop_front();
			TaskState state;
			{
				unlock_guard unlock(mtx);
				state = (*task.taskid)();
			}
			switch (state) {
			case TaskState::YIELD:
				if (!isExit) queuedTasks.emplace_back(std::move(task));
				break;
			case TaskState::END:
				tasks.erase(task.taskid);
				break;
			case TaskState::AWAIT:
				break;
			}
		}
	}
}

ExecutionThread::TaskID::TaskID(std::list<Task>::iterator taskid) : taskid(taskid) {}

bool ExecutionThread::resumeTask(const TaskID& taskID) {
	if (!isExit) {
		queuedTasks.push_back(taskID);
		return true;
	}
	return false;
}

} /* namespace threadutils */
} /* namespace dse */
