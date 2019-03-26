/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 ${user}.
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
 * Terminal.private.h
 *
 *  Created on: 5 авг. 2018 г.
 *      Author: disba1ancer
 */

#ifndef TERMINAL_PRIV_H_
#define TERMINAL_PRIV_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <array>
#include <vector>
#include <utility>
#include <iostream>
#include "../dse/Terminal.h"
#include "../dse/CloseEvent.h"
#include "../dse/ResizeEvent.h"
#include "../dse/observer.h"
#include "../dse/KeyEvent.h"
#include "../dse/RedrawEvent.h"

namespace {

using namespace dse::core;

struct TerminalPrivate {
	TerminalPrivate();
	~TerminalPrivate();
	static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static ATOM getWindowClsID();
	void notifyObservers(Event& event, Terminal::EventType type);

	static constexpr int GWLP_THIS = 0;
	struct {
		HWND hWnd;
		HDC hdc;
	} sysDepID; // System dependent identifier
	//std::array<std::vector<std::pair<util::Observer*, util::handler_t>>, Terminal::EVENT_COUNT> events;
	std::array<ObserverStore<void(Event*)>, Terminal::EVENT_COUNT> events;
};

typedef TerminalPrivate PRIVATE_CLASSNAME;

inline TerminalPrivate::TerminalPrivate() : sysDepID{CreateWindowEx(0, reinterpret_cast<LPCTSTR>(getWindowClsID()), 0,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		0, 0, static_cast<HINSTANCE>(GetModuleHandle(0)), this), 0} {
	if (sysDepID.hWnd == 0) {
		throw std::runtime_error("Fail to create window");
	}
	sysDepID.hdc = GetDC(sysDepID.hWnd);
}

inline TerminalPrivate::~TerminalPrivate() {
	DestroyWindow(sysDepID.hWnd);
}

inline LRESULT CALLBACK TerminalPrivate::staticWndProc(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam) {
	TerminalPrivate *aThis;
	if (message == WM_NCCREATE){
		aThis = reinterpret_cast<TerminalPrivate*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_THIS, reinterpret_cast<LONG_PTR>(aThis));
	} else {
		aThis = reinterpret_cast<TerminalPrivate*>(GetWindowLongPtr(hWnd, GWLP_THIS));
	}
	if(aThis == 0) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return aThis->wndProc(hWnd, message, wParam, lParam);
}

inline LRESULT TerminalPrivate::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT: {
		/*MSG msg;
		while(PeekMessage(&msg, 0, WM_TIMER, WM_TIMER, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}*/
		/*ValidateRect(sysDepID.hWnd, nullptr);
		while(PeekMessage(&msg, 0, WM_PAINT, WM_PAINT, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		InvalidateRect(sysDepID.hWnd, nullptr, FALSE);*/
		PAINTSTRUCT ps;
		BeginPaint(sysDepID.hWnd, &ps);
		RedrawEvent event;
		notifyObservers(event, Terminal::EVENT_ON_REDRAW);
		EndPaint(sysDepID.hWnd, &ps);
	}
		break;
	case WM_CLOSE: {
		CloseEvent event;
		notifyObservers(event, Terminal::EVENT_ON_CLOSE);
	}
		break;
	case WM_SIZE: {
		ResizeEvent event{LOWORD(lParam), HIWORD(lParam)};
		notifyObservers(event, Terminal::EVENT_ON_RESIZE);
	}
		break;
	//case WM_SYSKEYDOWN:
	case WM_KEYDOWN: {
		KeyEvent::State state = KeyEvent::DOWN;
		if (lParam & 0x40000000) state = KeyEvent::PRESSED;
		KeyEvent event(state, wParam);
		notifyObservers(event, Terminal::EVENT_ON_KEY_INPUT);
	}
		break;
	//case WM_SYSKEYUP:
	case WM_KEYUP: {
		KeyEvent event(KeyEvent::UP, wParam);
		notifyObservers(event, Terminal::EVENT_ON_KEY_INPUT);
	}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

inline ATOM TerminalPrivate::getWindowClsID() {
	static ATOM clsID = 0;
	if (clsID == 0) {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_OWNDC;
		wcex.lpfnWndProc = &staticWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 8;
		wcex.hInstance = static_cast<HINSTANCE>(GetModuleHandle(0));
		wcex.hIcon = LoadIcon(0, static_cast<LPCTSTR>(IDI_APPLICATION));
		wcex.hCursor = LoadCursor(0, static_cast<LPCTSTR>(IDC_ARROW));
		wcex.hbrBackground = NULL; //static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = TEXT("Terminal");
		wcex.hIconSm = LoadIcon(0, static_cast<LPCTSTR>(IDI_APPLICATION));

		clsID = RegisterClassEx(&wcex);
		if (clsID == 0){
			throw std::runtime_error("Fail to register window class");
		}
	}
	return clsID;
}

inline void TerminalPrivate::notifyObservers(Event& event, Terminal::EventType type) {
	/*for (auto& handlerPair : events[type]) {
		handlerPair.second(handlerPair.first, &event);
	}*/
	events.at(type)(&event);
}

} //namespace

#endif /* TERMINAL_PRIV_H_ */
