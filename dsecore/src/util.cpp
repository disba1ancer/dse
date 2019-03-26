/*
 * util.cpp
 *
 *  Created on: 17 мар. 2019 г.
 *      Author: Anton
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../dse/util.h"
#include "../dse/Terminal.h"

namespace dse {
namespace util {

int mainLoop(void (*func)(const void *), const void *data) {
	MSG msg;
	if (func) {
		while (true) {
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					return static_cast<int>(msg.wParam);
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} else {
				func(data);
			}
		}
	} else {
		BOOL retcode;
		while ((retcode = GetMessage(&msg, 0, 0, 0))) {
			if (retcode == -1) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return -1;
}

void returnMainLoop(int returnValue) {
	PostQuitMessage(returnValue);
}

}
}
