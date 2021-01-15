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

using dse::util::from_method;

void CustomPainter::paint(dse::os::WndEvtDt data) {
	auto dc = swal::PaintDC::BeginPaint(data.hWnd);
	RECT rc;
	rc.top = 8;
	rc.left = 8;
	rc.bottom = 100;
	rc.right = 100;
	dc.FillRect(rc, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
}

CustomPainter::CustomPainter(dse::os::Window &wnd) : wnd(&wnd),
	paintCon(wnd.subscribePaintEvent(from_method<&CustomPainter::paint>(*this))) {
}

void CustomPainter::invalidate() {
	InvalidateRect(wnd->getSysData().hWnd, nullptr, FALSE);
	UpdateWindow(wnd->getSysData().hWnd);
}
