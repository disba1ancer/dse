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
	while (m_isRun && !(isExit && tasks.empty())) {
		if (!tasks.empty()){
			auto& task = tasks.front();
			bool repeat;
			{
				unlock_guard unlock(mtx);
				repeat = task();
			}
			if (repeat && !isExit) tasks.emplace_back(std::move(task));
			tasks.pop_front();
		} else {
			cond_var.wait(lock);
		}
	}
	tasks.clear();
	isExit = false;
	m_isRun = false;
}

void ExecutionThread::run() {
	m_isRun = true;
	result = -1;
	thread = std::thread(&threadEntry, this);
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

bool ExecutionThread::addTask(std::function<bool()> &&task) {
	std::lock_guard lock(mtx);
	if (!isExit) {
		tasks.emplace_back(std::move(task));
		cond_var.notify_one();
		return true;
	}
	return false;
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

} /* namespace threadutils */
} /* namespace dse */
