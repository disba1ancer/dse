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
	ThreadData& thrData = (thrDataPtr ? *thrDataPtr : threadsData[0]);
	if (task.state == TaskState::Await) {
		task.state = TaskState::Ready;
	}
	std::scoped_lock lck(thrData.mtx);
	thrData.taskQueue.push_back(&task);
	condvar.notify_all();
	if (mainThread) PostThreadMessage(mainThread, WakeupMessage, 0, 0);
}

int ThreadPool_win32::run([[maybe_unused]] PoolCaps caps) {
	this->caps = caps;
	running.store(true, std::memory_order_relaxed);
	for (std::size_t i = 1; i < threadsData.size(); ++i) {
		threadsData[i].thr = std::thread(&ThreadPool_win32::thrEntry, this, i);
	}
	thrDataPtr = &(threadsData[0]);
	join(true);
	for (std::size_t i = 1; i < threadsData.size(); ++i) {
		threadsData[i].thr.join();
	}
	return 0;
}

void ThreadPool_win32::join() {
	join(false);
}

void ThreadPool_win32::stop() {
	if (running.exchange(false, std::memory_order_relaxed)) {
		condvar.notify_all();
		if (mainThread) swal::error::throw_or_result(PostThreadMessage(mainThread, WakeupMessage, 0, 0));
	}
}

std::weak_ptr<ThreadPool_win32> ThreadPool_win32::getCurrentPool() {
	auto t = thrDataPtr->currentPool->weak_from_this();
	return t;
}

void ThreadPool_win32::join(bool isMain) {
	if (thrDataPtr) {
		ThreadData& thrData = *thrDataPtr;
		while (pop(thrData, isMain)) {
			std::scoped_lock lck(thrData.currentTask->mtx);
			auto state = thrData.currentTask->state;
			/*if (state != TaskState::Canceled)*/ {
				unlock_guard ulck(thrData.currentTask->mtx);
				state = thrData.currentTask->taskHandler();
			}
			thrData.currentTask->state = state;
			switch (state) {
				case TaskState::Ready: {
					std::scoped_lock lck(thrData.mtx);
					thrData.taskQueue.push_back(thrData.currentTask);
					break;
				}
				case TaskState::End: {
					auto fHandler = std::move(thrData.currentTask->fHandler);
					fHandler();
				}
	//				[[fallthrough]];
	//			case TaskState::Canceled:
					break;
				case TaskState::Await:
					break;
			}
		}
	}
}

auto ThreadPool_win32::getCurrentTask() -> Task* {
	return thrDataPtr->currentTask;
}

bool ThreadPool_win32::pop(ThreadData& thrData, bool isMain) {
	while (running.load(std::memory_order_relaxed)) {
		if (isMain) {
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				switch (msg.message) {
					case WM_QUIT: {
						running.store(false, std::memory_order_relaxed);
						return false;
					}
					case WakeupMessage:
						break;
					default:
						TranslateMessage(&msg);
						DispatchMessage(&msg);
				}
			}
		}
		{
			std::scoped_lock lck(thrData.mtx);
			if (thrData.taskQueue.empty()) {
				for (auto& thrData2 : threadsData) {
					if (thrData2.mtx.try_lock()) {
						std::scoped_lock lck(std::adopt_lock, thrData2.mtx);
						auto beg = thrData2.taskQueue.begin();
						auto end = beg + thrData2.taskQueue.size() / 2;
						if (beg != end) {
							thrData.taskQueue.insert(thrData.taskQueue.end(), beg, end);
							break;
						}
					}
				}
			}
			if (!thrData.taskQueue.empty()){
				thrData.currentTask = thrData.taskQueue.front();
				thrData.taskQueue.pop_front();
				return true;
			}
		}
		if (isMain) {
			swal::error::throw_or_result(WaitMessage());
		} else {
			std::unique_lock cvlck(cvmtx);
			condvar.wait(cvlck);
		}
	}
	return false;
}

void ThreadPool_win32::thrEntry(size_t dataIdx) {
	thrDataPtr = &(threadsData[dataIdx]);
	join();
}

} /* namespace dse::threadutils */
