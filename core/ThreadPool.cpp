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

ThreadPool::ThreadPool(unsigned int concurrency) : impl(this, concurrency)
{}

ThreadPool::~ThreadPool()
{}

void ThreadPool::Schedule(const Task& task)
{
	impl->Schedule(task);
}

int ThreadPool::Run(PoolCaps caps)
{
	return impl->Run(this, caps);
}

void  ThreadPool::Join()
{
	impl->Join();
}

void ThreadPool::Stop()
{
	impl->Stop();
}

auto ThreadPool::GetCurrentTask() -> const Task&
{
	return ThreadPool_impl::GetCurrentTask();
}

ThreadPool* ThreadPool::GetCurrentPool()
{
	return ThreadPool_impl::GetCurrentPool();
}

} /* namespace dse::core */
