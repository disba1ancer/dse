/*
 * CustomPainter.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include <dse/core/win32.h>
#include <swal/gdi.h>
#include <dse/core/WindowEventData_win32.h>
#include <dse/core/WindowData_win32.h>
#include <dse/util/functional.h>
#include "CustomPainter.h"

namespace {
auto getWindowHandle(dse::core::Window* window) -> swal::Wnd {
	return window->GetSysData().hWnd;
}
}

using dse::util::StaticMemFn;

void CustomPainter::paint(dse::core::WndEvtDt data) {
	auto wnd = swal::Wnd(data.hWnd);
	auto dc = wnd.BeginPaint();
	RECT rc;
	rc.top = 8;
	rc.left = 8;
	rc.bottom = 100;
	rc.right = 100;
	dc.FillRect(rc, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
}

CustomPainter::CustomPainter(dse::core::Window &wnd) : wnd(&wnd),
	paintCon(wnd.SubscribePaintEvent(StaticMemFn<&CustomPainter::paint>(*this))) {
}

void CustomPainter::invalidate() {
	auto wnd = getWindowHandle(this->wnd);
	wnd.InvalidateRect(false);
	wnd.UpdateWindow();
}
