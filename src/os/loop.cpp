/*
 * loop.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifdef _WIN32
#include "win32.h"
#endif

#include "../threadutils/ExecutionThread.h"
#include <iostream>
#include "loop.h"
#include <utility>

using dse::threadutils::ExecutionThread;

namespace {
class NonLockLoopTask {
	UINT_PTR timID;
public:
	NonLockLoopTask() : timID(0) {}
	NonLockLoopTask(const NonLockLoopTask&) : NonLockLoopTask() {}
	NonLockLoopTask(NonLockLoopTask&& src) : timID(src.timID){
		src.timID = 0;
	}
	NonLockLoopTask& operator=(const NonLockLoopTask&) {
		timID = 0;
		return *this;
	}
	NonLockLoopTask& operator=(NonLockLoopTask&& src) {
		std::swap(timID, src.timID);
		return *this;
	}
	~NonLockLoopTask() { if (timID) KillTimer(NULL, timID); }
	static void CALLBACK yieldTimer(HWND, UINT, UINT_PTR, DWORD) {
		ExecutionThread::getCurrentThread().yieldTasks();
	}
	bool operator()() {
		MSG msg;
		if (!timID) {
			timID = SetTimer(NULL, 0, 10, yieldTimer);
		}
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				ExecutionThread::getCurrentThread().exit(msg.wParam);
				return false;
			}
			if (!(msg.message == WM_TIMER || msg.wParam == timID)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return true;
	}
};
}

std::function<bool()> dse::os::nonLockLoop() {
	return NonLockLoopTask();
}
