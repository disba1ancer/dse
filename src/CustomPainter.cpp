/*
 * CustomPainter.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "os/win32.h"
#include <swal/gdi.h>
#include "os/WindowEventData_win32.h"
#include "os/WindowData_win32.h"
#include "util/functional.h"
#include "CustomPainter.h"

namespace {
auto getWindowHandle(dse::os::Window* window) -> swal::Wnd {
	return window->GetSysData().hWnd;
}
}

using dse::util::from_method;

void CustomPainter::paint(dse::os::WndEvtDt data) {
	auto wnd = swal::Wnd(data.hWnd);
	auto dc = wnd.BeginPaint();
	RECT rc;
	rc.top = 8;
	rc.left = 8;
	rc.bottom = 100;
	rc.right = 100;
	dc.FillRect(rc, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
}

CustomPainter::CustomPainter(dse::os::Window &wnd) : wnd(&wnd),
	paintCon(wnd.SubscribePaintEvent(from_method<&CustomPainter::paint>(*this))) {
}

void CustomPainter::invalidate() {
	auto wnd = getWindowHandle(this->wnd);
	wnd.InvalidateRect(false);
	wnd.UpdateWindow();
}
