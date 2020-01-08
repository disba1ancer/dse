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

using dse::threadutils::ExecutionThread;

bool dse::os::nonLockLoop() {
	MSG msg;
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			ExecutionThread::getCurrentThread().exit(msg.wParam);
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}
