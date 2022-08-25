/*
 * ThreadPool.cpp
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#include <dse/core/ThreadPool.h>

#ifdef _WIN32
#include "ThreadPool_win32.h"
#endif

namespace dse::core {

ThreadPool::ThreadPool(unsigned int concurrency) : pimpl(std::make_shared<ThreadPool_impl>(concurrency)) {
}

ThreadPool::ThreadPool(const std::shared_ptr<ThreadPool_impl> &ptr) : pimpl(ptr) {
}

ThreadPool::ThreadPool(std::nullptr_t) noexcept : pimpl() {
}

/*ThreadPool::~ThreadPool() {
}*/

void ThreadPool::Schedule(const Task& task) {
	GetImpl()->Schedule(task);
}

int ThreadPool::Run(PoolCaps caps) {
	return GetImpl()->Run(caps);
}

void  ThreadPool::Join() {
	GetImpl()->Join();
}

void ThreadPool::Stop() {
	GetImpl()->Stop();
}

auto ThreadPool::GetCurrentTask() -> const Task& {
	return ThreadPool_impl::GetCurrentTask();
}

ThreadPool ThreadPool::GetCurrentPool()
{
	return ThreadPool_impl::GetCurrentPool();
}

//ThreadPool::Task::Task(TaskHandler&& taskHandler) :
//	taskHandler(std::move(taskHandler))
//{}

///*void ThreadPool::Task::cancel() {
//	std::scoped_lock lck(mtx);
//	state = TaskState::Canceled;
//}*/

//void ThreadPool::Task::then(FinishHandler &&fHandler) {
//	this->fHandler = std::move(fHandler);
//}

//void ThreadPool::Task::reset(ThreadPool::TaskHandler &&taskHandler)
//{
//	fHandler = nullptr;
//	this->taskHandler = std::move(taskHandler);
//}

} /* namespace dse::core */
