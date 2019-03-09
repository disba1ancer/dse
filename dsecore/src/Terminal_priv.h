/*
 * Terminal.private.h
 *
 *  Created on: 5 авг. 2018 г.
 *      Author: Anton
 */

#ifndef TERMINAL_PRIV_H_
#define TERMINAL_PRIV_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <array>
#include <vector>
#include <utility>
#include <iostream>
#include <dse/Terminal.h>
#include <dse/CloseEvent.h>
#include <dse/ResizeEvent.h>
#include <dse/observer.h>
#include <dse/KeyEvent.h>

namespace {

using namespace dse::core;

struct TerminalPrivate {
	TerminalPrivate();
	~TerminalPrivate();
	static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static ATOM getWindowClsID();
	void notifyObservers(util::Event& event, Terminal::EventType type);

	static constexpr int GWLP_THIS = 0;
	struct {
		HWND hWnd;
		HDC hdc;
	} sysDepID; // System dependent identifier
	//std::array<std::vector<std::pair<util::Observer*, util::handler_t>>, Terminal::EVENT_COUNT> events;
	std::array<util::ObserverStore<void(util::Event&)>, Terminal::EVENT_COUNT> events;
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
	case WM_PAINT:
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

inline void TerminalPrivate::notifyObservers(util::Event& event, Terminal::EventType type) {
	/*for (auto& handlerPair : events[type]) {
		handlerPair.second(handlerPair.first, &event);
	}*/
	events.at(type)(event);
}

} //namespace

#endif /* TERMINAL_PRIV_H_ */
