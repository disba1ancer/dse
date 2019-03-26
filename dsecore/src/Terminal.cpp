/*
 * Terminal.cpp
 *
 *  Created on: 23 авг. 2017 г.
 *      Author: Anton
 */

#include "../dse/Terminal.h"
#include "../dse/util.h"
#include "Terminal_priv.h"

namespace dse {
namespace core {

Terminal::Terminal() {
	//static_assert(sizeof(storage) >= sizeof(TerminalPrivate), "Terminal.private is very long");
	this->priv = new PRIVATE_CLASSNAME{};
	//auto& pthis = PRIVATE;
}

Terminal::~Terminal() {
	auto& pthis = PRIVATE;

	delete &pthis;
}

const void* Terminal::getSysDepId() const {
	auto& pthis = PRIVATE;
	return &(pthis.sysDepID);
}

bool Terminal::isVisible() const {
	auto& pthis = PRIVATE;
	return IsWindowVisible(pthis.sysDepID.hWnd);
}

void Terminal::show(ShowCommand command) {
	auto& pthis = PRIVATE;
	int showCmd;
	switch (command) {
	case HIDE:
		showCmd = SW_HIDE;
		break;
	case SHOW:
		showCmd = SW_SHOW;
		break;
	case SHOW_MINIMIZED:
		showCmd = SW_SHOWMINIMIZED;
		break;
	case (SHOW_RESTORED):
		showCmd = SW_SHOWNORMAL;
		break;
	case SHOW_MAXIMIZED:
		showCmd = SW_SHOWMAXIMIZED;
		break;
	case SHOW_FULL_SCREEN:
		showCmd = SW_SHOWMAXIMIZED;
		break;
	}
	auto style = GetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE);
	if (command == SHOW_FULL_SCREEN && !(style & WS_POPUP)) {
		style |= WS_POPUP;
		style &= ~WS_OVERLAPPEDWINDOW;
		SetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE, style);
	} else if(style & WS_POPUP) {
		style &= ~WS_POPUP;
		style |= WS_OVERLAPPEDWINDOW;
		SetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE, style);
	}
	ShowWindow(pthis.sysDepID.hWnd, showCmd);
}

int Terminal::getHeight() const {
	auto& pthis = PRIVATE;
	RECT rc;
	GetClientRect(pthis.sysDepID.hWnd, &rc);
	return rc.bottom;
}

int Terminal::getWidth() const {
	auto& pthis = PRIVATE;
	RECT rc;
	GetClientRect(pthis.sysDepID.hWnd, &rc);
	return rc.right;
}

bool Terminal::isSizable() const {
	auto& pthis = PRIVATE;
	return bool(GetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE) & WS_SIZEBOX);
}

void Terminal::resize(int width, int height) {
	auto& pthis = PRIVATE;
	RECT rc{};
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRect(&rc, static_cast<LONG>(GetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE)), FALSE);
	SetWindowPos(pthis.sysDepID.hWnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
}

void Terminal::makeSizable(bool sizable) {
	auto& pthis = PRIVATE;
	SetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE, (GetWindowLongPtr(pthis.sysDepID.hWnd, GWL_STYLE) & ~WS_SIZEBOX) | (sizable ? WS_SIZEBOX : 0));
}

Connection<void> Terminal::attach(EventType eventType,
		void* owner, void (*callback)(void*, Event*)) {
	auto& pthis = PRIVATE;
	return pthis.events.at(eventType).add(owner, callback);
}

void Terminal::close() {
	auto& pthis = PRIVATE;
	CloseWindow(pthis.sysDepID.hWnd);
}

void Terminal::forceRedraw() {
	auto& pthis = PRIVATE;
	InvalidateRect(pthis.sysDepID.hWnd, nullptr, FALSE);
}


} /* namespace core */
} /* namespace dse */
