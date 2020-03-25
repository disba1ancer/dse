/*
 * mcursor.cpp
 *
 *  Created on: 25 мар. 2020 г.
 *      Author: disba1ancer
 */

#include "mcursor.h"
#include "WindowData_win32.h"
#include "win32.h"

void dse::os::setMouseCursorPos(const math::ivec2 &pos) {
	SetCursorPos(pos[0], pos[1]);
}

void dse::os::setMouseCursorPosWndRel(const math::ivec2 &pos, Window &wnd) {
	auto wndData = wnd.getSysData();
	POINT pt = { 0, 0 };
	ClientToScreen(wndData.hWnd, &pt);
	setMouseCursorPos(math::ivec2{pt.x, pt.y} + pos);
}
