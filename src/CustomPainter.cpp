/*
 * CustomPainter.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "os/PaintEventData_win32.h"
#include "os/WindowData_win32.h"
#include "notifier/make_handler.h"
#include "CustomPainter.h"

using dse::notifier::make_handler;

void CustomPainter::paint(dse::os::WndEvtDt, dse::os::PntEvtDt pd) {
	auto& hdc = pd.hdc;
	RECT rc;
	rc.top = 8;
	rc.left = 8;
	rc.bottom = 100;
	rc.right = 100;
	FillRect(hdc, &rc, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
}

CustomPainter::CustomPainter(dse::os::Window &wnd) : wnd(&wnd),
		paintCon(wnd.subscribePaintEvent(make_handler<&paint>(this))) {
}

void CustomPainter::invalidate() {
	InvalidateRect(wnd->getSysData().hWnd, nullptr, FALSE);
	UpdateWindow(wnd->getSysData().hWnd);
}
