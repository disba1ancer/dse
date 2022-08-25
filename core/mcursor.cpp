/*
 * mcursor.cpp
 *
 *  Created on: 25 мар. 2020 г.
 *      Author: disba1ancer
 */

#include <dse/core/mcursor.h>
#include <dse/core/WindowData_win32.h>
#include <dse/core/win32.h>
#include <dse/math/vmath.h>

void dse::core::setMouseCursorPos(const math::ivec2 &pos) {
	SetCursorPos(pos[0], pos[1]);
}

void dse::core::SetMouseCursorPosWndRel(const math::ivec2 &pos, Window &wnd) {
	auto wndData = wnd.GetSysData();
	POINT pt = { 0, 0 };
	ClientToScreen(wndData.hWnd, &pt);
	setMouseCursorPos(math::ivec2{pt.x, pt.y} + pos);
}
