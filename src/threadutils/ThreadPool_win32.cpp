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

namespace dse {
namespace threadutils {

ThreadPool_win32::ThreadPool_win32() {
}

ThreadPool_win32::~ThreadPool_win32() {
	stop(true);
}

void ThreadPool_win32::schedule(Task &task) {
	{
		std::scoped_lock lck(task.mtx);
		if (task.state == TaskState::Await) {
			task.state = TaskState::Ready;
		}
	}
	{
		std::scoped_lock lck(mtx);
		scheduledTasks.push_back(&task);
		condvar.notify_all();
		if (mainThread) PostThreadMessage(mainThread, WakeupMessage, 0, 0);
	}
}

int ThreadPool_win32::join(ThreadType type) {
	{
		std::scoped_lock lck(mtx);
		if (type == ThreadType::Main) {
			if (mainThread == NULL) {
				mainThread = GetCurrentThreadId();
			} else {
				throw std::runtime_error("Only one main thread are allowed");
			}
		}
		if (!running && refs) {
			return -1;
		}
		running = true;
		++refs;
	}
	Task* task;
	while (pop(task, type)) {
		std::scoped_lock lck(task->mtx);
		auto state = task->state;
		if (state != TaskState::Canceled) {
			unlock_guard ulck(task->mtx);
			state = task->taskHandler();
		}
		task->state = state;
		switch (state) {
		case TaskState::Ready: {
			std::scoped_lock lck(mtx);
			scheduledTasks.push_back(task);
			break;
		}
		case TaskState::End:
			task->fHandler();
			[[fallthrough]];
		case TaskState::Canceled:
			break;
		case TaskState::Await:
			break;
		}
	}
	{
		std::scoped_lock lck(mtx);
		--refs;
		if (!refs) {
			running = false;
		}
	}
	return 0;
}

void ThreadPool_win32::stop(bool wait) {
	{
		std::scoped_lock lck(mtx);
		running = false;
		condvar.notify_all();
		if (mainThread) PostThreadMessage(mainThread, WakeupMessage, 0, 0);
	}
	while(wait && bool(util::access(mtx, refs))) {}
}

bool ThreadPool_win32::pop(Task*& task, ThreadType type) {
	MSG msg;
	std::scoped_lock lck(mtx);
	while (running) {
		if (type == ThreadType::Main) {
			unlock_guard ulck(mtx);
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (!scheduledTasks.size()) {
			unlock_guard ulck(mtx);
			if (type == ThreadType::Main) {
				WaitMessage();
			} else {
				std::unique_lock cvlck(cvmtx);
				condvar.wait(cvlck);
			}
		} else {
			task = scheduledTasks.front();
			scheduledTasks.pop_front();
			break;
		}
	}
	return running;
}

} /* namespace threadutils */
} /* namespace dse */
