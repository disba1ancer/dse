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

namespace dse::threadutils {

ThreadPool::ThreadPool(unsigned int concurrency) : pimpl(std::make_shared<ThreadPool_impl>(concurrency)) {
}

ThreadPool::ThreadPool(const std::shared_ptr<ThreadPool_impl> &ptr) : pimpl(ptr) {
}

ThreadPool::ThreadPool(std::nullptr_t) noexcept : pimpl() {
}

/*ThreadPool::~ThreadPool() {
}*/

void ThreadPool::schedule(Task &task) {
	get_impl()->schedule(task);
}

int ThreadPool::run(PoolCaps caps) {
	return get_impl()->run(caps);
}

void  ThreadPool::join() {
	get_impl()->join();
}

void ThreadPool::stop() {
	get_impl()->stop();
}

ThreadPool::Task *ThreadPool::getCurrentTask()
{
	return ThreadPool_impl::getCurrentTask();
}

ThreadPool ThreadPool::getCurrentPool()
{
	return ThreadPool_impl::getCurrentPool();
}

ThreadPool::Task::Task(TaskHandler&& taskHandler) :
	taskHandler(std::move(taskHandler))
{}

/*void ThreadPool::Task::cancel() {
	std::scoped_lock lck(mtx);
	state = TaskState::Canceled;
}*/

void ThreadPool::Task::then(FinishHandler &&fHandler) {
	this->fHandler = std::move(fHandler);
}

void ThreadPool::Task::reset(ThreadPool::TaskHandler &&taskHandler)
{
	fHandler = nullptr;
	this->taskHandler = std::move(taskHandler);
}

} /* namespace dse::threadutils */
