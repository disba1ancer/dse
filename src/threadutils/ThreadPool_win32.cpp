/*
 * ThreadPoolwin32.cpp
 *
 *  Created on: 12 июл. 2020 г.
 *      Author: disba1ancer
 */

#include "ThreadPool_win32.h"
#include "unlock_guard.h"
#include "spinlock.h"
#include "util/Access.h"
#include "util/FinalStep.h"
#include <iostream>
#include "dse_config.h"

namespace dse::threadutils {

//thread_local std::weak_ptr<ThreadPool_win32> ThreadPool_win32::currentPool;
//thread_local ThreadPool::Task* ThreadPool_win32::currentTask;
thread_local ThreadPool_win32::ThreadData* ThreadPool_win32::thrDataPtr;

ThreadPool_win32::ThreadPool_win32(unsigned int concurrency) : threadsData(concurrency) {
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

void ThreadPool_win32::schedule(Task &task) {
	std::size_t taskCount;
	ThreadData& thrData = (thrDataPtr ? *thrDataPtr : threadsData[0]);
	{
		std::scoped_lock lck(thrData.mtx);
		thrData.taskQueue.push_back(&task);
		taskCount = thrData.taskQueue.size();
	}
	//condvar.notify_all();
	if (slpThrds.load(std::memory_order_relaxed) && taskCount > 1) {
		slpThrds.store(false, std::memory_order_relaxed);
		wakeupThreads();
	}
}

int ThreadPool_win32::run(PoolCaps caps) {
	this->caps = caps;
	util::FinalStep f([this]{
		thrDataPtr = nullptr;
		for (std::size_t i = 1; i < threadsData.size(); ++i) {
			threadsData[i].thr.join();
		}
		running.store(false, std::memory_order_relaxed);
	});

	running.store(true, std::memory_order_relaxed);
	for (std::size_t i = 1; i < threadsData.size(); ++i) {
		threadsData[i].thr = std::thread(&ThreadPool_win32::thrEntry, this, i);
	}

	threadsData[0].currentPool = this;
	thrDataPtr = &(threadsData[0]);
	join(true);

	return 0;
}

void ThreadPool_win32::join() {
	join(false);
}

void ThreadPool_win32::stop() {
	if (running.exchange(false, std::memory_order_relaxed)) {
		wakeupThreads();
	}
}

std::shared_ptr<ThreadPool_win32> ThreadPool_win32::getCurrentPool() {
	auto t = thrDataPtr->currentPool->shared_from_this();
	return t;
}

void ThreadPool_win32::iocpAttach(swal::Handle &handle) {
	iocp.AssocFile(handle, 0x0DB9);
}

void ThreadPool_win32::join(bool isMain) {
	if (thrDataPtr) {
		ThreadData& thrData = *thrDataPtr;
		thrData.isMain = isMain;
		while (running.load(std::memory_order_relaxed)) {
			handleSystem(thrData);
			for (
				int i = 0;
				i < DSE_THREADPOOL_TASKS_PER_IO
					&& (thrData.currentTask = pop(thrData));
				++i
			) {
				if (thrData.currentTask->taskHandler() == TaskState::End) {
					thrData.currentTask->fHandler();
				}
			}
			waitForWork(thrData);
		}
	}
}

auto ThreadPool_win32::getCurrentTask() -> Task* {
	return thrDataPtr->currentTask;
}

void ThreadPool_win32::thrEntry(size_t dataIdx) {
	threadsData[dataIdx].currentPool = this;
	thrDataPtr = &(threadsData[dataIdx]);
	join();
}

void ThreadPool_win32::handleMessages() {
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

void ThreadPool_win32::trySteal(ThreadData& thrData) {
	bool lckQueues = false;
	for (;;) {
		for (auto& thrData2 : threadsData) {
			if (&thrData != &thrData2) {
				if (thrData2.mtx.try_lock()) {
					std::scoped_lock lck(std::adopt_lock, thrData2.mtx);
					auto beg = thrData2.taskQueue.begin();
					auto end = beg + thrData2.taskQueue.size() / 2;
					if (beg != end) {
						thrData.taskQueue.insert(thrData.taskQueue.end(), beg, end);
						return;
					}
				} else {
					lckQueues = true;
				}
			}
		}
		if (lckQueues) {
			unlock_guard lck(thrData.mtx);
			handleSystem(thrData);
			lckQueues = false;
		} else {
			return;
		}
	}
}

auto ThreadPool_win32::pop(ThreadData &thrData) -> Task* {
	std::scoped_lock lck(thrData.mtx);
	if (thrData.taskQueue.empty()) {
		trySteal(thrData);
	}
	if (!thrData.taskQueue.empty()){
		auto task = thrData.taskQueue.front();
		thrData.taskQueue.pop_front();
		return task;
	}
	return nullptr;
}

void ThreadPool_win32::wakeupThreads() {
	for (std::size_t i = 0; i < threadsData.size(); ++i) {
		iocp.PostQueuedCompletionStatus(0, WakeupMessage, nullptr);
	}
	if (mainThread) PostThreadMessage(mainThread, WakeupMessage, 0, 0);
}

void ThreadPool_win32::handleIO(ThreadPool_win32::ThreadData &thrData) {
	auto& ioCompletion = thrData.ioCompletion;
	do {
		if (
			ioCompletion.ovl != nullptr ||
			ioCompletion.error != WAIT_TIMEOUT
		) ioCompletion = iocp.GetQueuedCompletionStatus(0);
		if (ioCompletion.ovl != nullptr) {
			auto asyncHandle = static_cast<IAsyncIO*>(ioCompletion.ovl);
			asyncHandle->complete(ioCompletion.bytesTransfered, ioCompletion.error);
		}
	} while (
		ioCompletion.ovl != nullptr ||
		ioCompletion.error != WAIT_TIMEOUT
	);
}

void ThreadPool_win32::waitForWork(ThreadData &thrData) {
	if (getTasksCount(thrData) == 0) {
		slpThrds.store(true, std::memory_order_relaxed);
		if (thrData.isMain) {
			swal::error::throw_or_result(WaitMessage());
		} else {
			thrData.ioCompletion = iocp.GetQueuedCompletionStatus(INFINITE);
		}
	}
}

size_t ThreadPool_win32::getTasksCount(ThreadData &thrData) {
	std::scoped_lock lck(thrData.mtx);
	return thrData.taskQueue.size();
}

void ThreadPool_win32::handleSystem(ThreadData& thrData) {
	if (thrData.isMain) handleMessages();
	handleIO(thrData);
}

std::shared_ptr<ThreadPool_win32> IAsyncIO::getImplFromPool(ThreadPool& pool) {
	return pool.get_impl();
}

} /* namespace dse::threadutils */
