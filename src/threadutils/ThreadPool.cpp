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

ThreadPool::ThreadPool() : impl(std::make_unique<ThreadPool_impl>()) {
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::schedule(Task &task) {
	impl->schedule(task);
}

int ThreadPool::join(ThreadType type) {
	return impl->join(type);
}

void ThreadPool::stop(bool wait) {
	impl->stop(wait);
}

ThreadPool::Task::Task(TaskHandler&& taskHandler) :
	taskHandler(std::move(taskHandler))
{}

void ThreadPool::Task::cancel() {
	std::scoped_lock lck(mtx);
	state = TaskState::Canceled;
}

void ThreadPool::Task::then(FinishHandler &&fHandler) {
	std::scoped_lock lck(mtx);
	this->fHandler = std::move(fHandler);
}

} /* namespace threadutils */
} /* namespace dse */
