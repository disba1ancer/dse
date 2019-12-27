/*
 * loop.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: Anton
 */

#include "loop.h"
#include "windows.h"
#include "../threadutils/ExecutionThread.h"

using dse::threadutils::ExecutionThread;

bool dse::os::nonLockLoop() {
	MSG msg;
	if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			ExecutionThread::getCurrentThread().exit(msg.wParam);
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}
