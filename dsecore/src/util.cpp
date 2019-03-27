/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 disba1ancer.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * util.cpp
 *
 *  Created on: 17 мар. 2019 г.
 *      Author: disba1ancer
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
