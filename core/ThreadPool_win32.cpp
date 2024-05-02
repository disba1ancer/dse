/*
 * ThreadPoolwin32.cpp
 *
 *  Created on: 12 июл. 2020 г.
 *      Author: disba1ancer
 */

#include "ThreadPool_win32.h"
#include <dse/util/unlock_guard.h>
#include <dse/util/access.h>
#include <dse/util/scope_exit.h>
#include <iostream>
#include <dse_config.h>

namespace dse::core {

namespace threadpool_detail {

extern thread_local swal::Event thrEvent{true, true};

}

//thread_local std::weak_ptr<ThreadPool_win32> ThreadPool_win32::currentPool;
//thread_local ThreadPool::Task* ThreadPool_win32::currentTask;
thread_local ThreadPool_win32::ThreadData* ThreadPool_win32::thrDataPtr;

ThreadPool_win32::ThreadPool_win32(unsigned int concurrency) /*: threadsData(concurrency)*/ {
	for (std::size_t i = 0; i < concurrency; ++i) {
		threadsData.emplace_back(ThreadData{});
	}
	if (!concurrency) {
		throw std::runtime_error("Unexpected concurrency value");
	}
	for (auto& thrData : threadsData) {
		thrData.currentPool = this;
	}
}

ThreadPool_win32::~ThreadPool_win32() {
	if (running.load(std::memory_order_relaxed)) {
		std::terminate();
	}
}

void ThreadPool_win32::Schedule(const Task& task) {
	std::size_t taskCount;
	ThreadData& thrData = (thrDataPtr ? *thrDataPtr : threadsData[0]);
	{
		std::scoped_lock lck(thrData.mtx);
		thrData.taskQueue.push_back(task);
		taskCount = thrData.taskQueue.size();
	}
	//condvar.notify_all();
	if (slpThrds.load(std::memory_order_relaxed) /*&& taskCount > 1*/) {
		slpThrds.store(false, std::memory_order_relaxed);
		WakeupThreads();
	}
}

int ThreadPool_win32::Run(PoolCaps caps) {
	this->caps = caps;
	util::scope_exit f([this]{
		thrDataPtr = nullptr;
		for (std::size_t i = 1; i < threadsData.size(); ++i) {
			threadsData[i].thr.join();
		}
		running.store(false, std::memory_order_relaxed);
	});

	running.store(true, std::memory_order_relaxed);
	mainThread = GetCurrentThreadId();
	for (std::size_t i = 1; i < threadsData.size(); ++i) {
		threadsData[i].thr = std::thread(&ThreadPool_win32::ThrEntry, this, i);
	}

	threadsData[0].currentPool = this;
	thrDataPtr = &(threadsData[0]);
	Join(true);

	return 0;
}

void ThreadPool_win32::Join() {
	Join(false);
}

void ThreadPool_win32::Stop() {
	if (running.exchange(false, std::memory_order_relaxed)) {
		WakeupThreads();
	}
}

std::shared_ptr<ThreadPool_win32> ThreadPool_win32::GetCurrentPool() {
	auto t = thrDataPtr->currentPool->shared_from_this();
	return t;
}

void ThreadPool_win32::IOCPAttach(swal::Handle &handle) {
	iocp.AssocFile(handle, 0x0DB9);
}

void ThreadPool_win32::Join(bool isMain) {
	if (thrDataPtr) {
		ThreadData& thrData = *thrDataPtr;
		thrData.isMain = isMain;
		while (running.load(std::memory_order_relaxed)) {
			HandleSystem(thrData);
			for (
				int i = 0;
				i < DSE_THREADPOOL_TASKS_PER_IO
					&& (thrData.currentTask = Pop(thrData));
				++i
			) {
				if (thrData.currentTask) {
					thrData.currentTask();
				}
			}
			WaitForWork(thrData);
		}
	}
}

auto ThreadPool_win32::GetCurrentTask() -> const Task& {
	return thrDataPtr->currentTask;
}

void ThreadPool_win32::ThrEntry(size_t dataIdx) {
	try {
		threadsData[dataIdx].currentPool = this;
		thrDataPtr = &(threadsData[dataIdx]);
		Join();
	} catch (std::exception& e) {
		std::cerr << "Error in thread " << dataIdx << ": " << e.what();
		std::terminate();
	} catch (...) {
		std::cerr << "Error in thread " << dataIdx << ": Exception with unknown base class";
		std::terminate();
	}
}

void ThreadPool_win32::HandleMessages() {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
			case WM_QUIT: {
				running.store(false, std::memory_order_relaxed);
				return;
			}
			case WakeupMessage:
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}
	}
}

void ThreadPool_win32::TrySteal(ThreadData& thrData) {
	while (true) {
		for (auto& thrData2 : threadsData) {
			if (&thrData != &thrData2) {
				if (thrData2.mtx.try_lock()) {
					std::scoped_lock lck(std::adopt_lock, thrData2.mtx);
					auto beg = thrData2.taskQueue.begin();
					auto size = thrData2.taskQueue.size();
					auto end = beg + size / 2;
					if (beg != end) {
						thrData.taskQueue.insert(thrData.taskQueue.end(), beg, end);
						thrData2.taskQueue.erase(beg, end);
						return;
					}
				} else {
					util::unlock_guard lck(thrData.mtx);
					HandleSystem(thrData);
					goto restartSteal;
				}
			}
		}
		return;
		restartSteal:;
	}
}

auto ThreadPool_win32::Pop(ThreadData &thrData) -> Task {
	std::scoped_lock lck(thrData.mtx);
	if (thrData.taskQueue.empty()) {
		TrySteal(thrData);
	}
	if (!thrData.taskQueue.empty()) {
		auto task = std::move(thrData.taskQueue.front());
		thrData.taskQueue.pop_front();
		return task;
	}
	return nullptr;
}

void ThreadPool_win32::WakeupThreads() {
	for (std::size_t i = 0; i < threadsData.size(); ++i) {
		iocp.PostQueuedCompletionStatus(0, WakeupMessage, nullptr);
	}
	if (mainThread) swal::winapi_call(PostThreadMessage(mainThread, WakeupMessage, 0, 0));
}

void ThreadPool_win32::HandleIO(ThreadPool_win32::ThreadData &thrData) {
	auto& ioCompletion = thrData.ioCompletion;
	for(;;) {
		if (ioCompletion.ovl != nullptr) {
			auto asyncHandle = static_cast<IAsyncIO*>(ioCompletion.ovl);
			asyncHandle->Complete(ioCompletion.bytesTransfered, ioCompletion.error);
		}
		ioCompletion = iocp.GetQueuedCompletionStatus(0);
		if (
			ioCompletion.ovl == nullptr &&
			ioCompletion.error == WAIT_TIMEOUT
		) break;
	}
}

void ThreadPool_win32::WaitForWork(ThreadData &thrData) {
	if (GetTasksCount(thrData) == 0 && running.load(std::memory_order_relaxed)) {
		slpThrds.store(true, std::memory_order_relaxed);
		if (thrData.isMain) {
			swal::winapi_call(WaitMessage());
		} else {
			thrData.ioCompletion = iocp.GetQueuedCompletionStatus(INFINITE);
		}
	}
}

size_t ThreadPool_win32::GetTasksCount(ThreadData &thrData) {
	std::scoped_lock lck(thrData.mtx);
	return thrData.taskQueue.size();
}

void ThreadPool_win32::HandleSystem(ThreadData& thrData) {
	if (thrData.isMain) HandleMessages();
	HandleIO(thrData);
}

std::shared_ptr<ThreadPool_win32> IAsyncIO::GetImplFromPool(ThreadPool& pool) {
	return pool.GetImpl();
}

} // namespace dse::core
