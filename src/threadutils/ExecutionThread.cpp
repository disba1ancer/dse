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
	ThreadMapLock mapLockObj(this);
	std::lock_guard mapLock(mapLockObj);
	std::unique_lock lock(mtx);
	while (m_isRun && !(isExit && tasks.empty())) {
		if (!tasks.empty()){
			auto& task = tasks.front();
			bool repeat;
			{
				unlock_guard<std::mutex> unlock(mtx);
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

void ExecutionThread::addTask(std::function<bool()> &&task) {
	std::lock_guard lock(mtx);
	if (!isExit) { // TODO: notify user about fail
		tasks.emplace_back(std::move(task));
		cond_var.notify_one();
	}
}

bool ExecutionThread::isRun() {
	std::lock_guard lock(mtx);
	return m_isRun;
}

bool ExecutionThread::getResult() {
	return result;
}

ExecutionThread::ThreadMapLock::ThreadMapLock(ExecutionThread* thread) : thread(thread) {
}

std::map<std::thread::id, ExecutionThread*> ExecutionThread::threads;

void ExecutionThread::ThreadMapLock::lock() {
	ExecutionThread::threads.insert(std::make_pair(std::this_thread::get_id(), thread));
}

void ExecutionThread::ThreadMapLock::unlock() {
	ExecutionThread::threads.erase(std::this_thread::get_id());
}

ExecutionThread& dse::threadutils::ExecutionThread::getCurrentThread() {
	return *(threads[std::this_thread::get_id()]);
}

} /* namespace threadutils */
} /* namespace dse */
