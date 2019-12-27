/*
 * ThreadPool.cpp
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "ThreadPool.h"
#include "unlock_guard.h"

namespace dse {
namespace threadutils {

void ThreadPool::poolEntry() {
	std::unique_lock lock(mtx);
	while (m_isRun) {
		if (!tasks.empty()){
			auto& task = tasks.front();
			bool repeat;
			{
				unlock_guard<std::mutex> unlock(mtx);
				repeat = task();
			}
			if (repeat) tasks.emplace_back(std::move(task));
			tasks.pop_front();
		} else {
			cond_var.wait(lock);
		}
	}
	tasks.clear();
	m_isRun = false;
}

void ThreadPool::startThreads(std::size_t num) {
	m_isRun = true;
	threads.clear();
	threads.resize(num);
	for (auto& thread : threads) {
		thread = std::thread(&poolEntry, this);
	}
}

void ThreadPool::terminate() {
	std::lock_guard lock(mtx);
	m_isRun = false;
}

void ThreadPool::join() {
	for (auto& thread : threads) {
		thread.join();
	}
}

void ThreadPool::addTask(std::function<bool()> &&task) {
	std::lock_guard lock(mtx);
	tasks.emplace_back(std::move(task));
}

bool ThreadPool::isRun() {
	std::lock_guard lock(mtx);
	return m_isRun;
}

} /* namespace threadutils */
} /* namespace dse */
