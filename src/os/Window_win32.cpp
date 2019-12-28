/*
 * Window_win32.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "Window_win32.h"
#include <exception>

namespace dse {
namespace os {

Window_win32::Window_win32() : hWnd(CreateWindowEx(0, reinterpret_cast<LPCTSTR>(makeWindowClsID()), 0,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		0, 0, static_cast<HINSTANCE>(GetModuleHandle(0)), this)) {
	if (hWnd == 0) {
		throw std::runtime_error("Fail to create window");
	}
}

LRESULT Window_win32::staticWndProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam) {
	Window_win32 *aThis;
	if (message == WM_NCCREATE){
		aThis = reinterpret_cast<Window_win32*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		hWndMap.insert(std::make_pair(hWnd, aThis));
	} else {
		auto iter = hWndMap.find(hWnd);
		if (iter == hWndMap.end()) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		aThis = iter->second;
	}
	return aThis->wndProc(hWnd, message, wParam, lParam);
}

LRESULT Window_win32::wndProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam) {
	switch(message) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		//RedrawEvent event;
		//notifyObservers(event, Terminal::EVENT_ON_REDRAW);
		EndPaint(hWnd, &ps);
	}
		break;
	case WM_CLOSE: {
		//CloseEvent event;
		//notifyObservers(event, Terminal::EVENT_ON_CLOSE);
		closeSubscribers.notify();
	}
		break;
	case WM_SIZE: {
		//ResizeEvent event{LOWORD(lParam), HIWORD(lParam)};
		//notifyObservers(event, Terminal::EVENT_ON_RESIZE);
	}
		break;
	case WM_KEYDOWN: {
		/*KeyEvent::State state = KeyEvent::DOWN;
		if (lParam & 0x40000000) state = KeyEvent::PRESSED;
		KeyEvent event(state, wParam, (lParam >> 16) & 0xFF);
		notifyObservers(event, Terminal::EVENT_ON_KEY_INPUT);*/
	}
		break;
	case WM_KEYUP: {
		//KeyEvent event(KeyEvent::UP, wParam, (lParam >> 16) & 0xFF);
		//notifyObservers(event, Terminal::EVENT_ON_KEY_INPUT);
	}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM Window_win32::makeWindowClsID() {
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
		wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = TEXT("dse.os.Window");
		wcex.hIconSm = LoadIcon(0, static_cast<LPCTSTR>(IDI_APPLICATION));

		clsID = RegisterClassEx(&wcex);
		if (clsID == 0){
			throw std::runtime_error("Fail to register window class");
		}
	}
	return clsID;
}

Window_win32::~Window_win32() {
	DestroyWindow(hWnd);
	hWndMap.erase(hWnd);
}

bool Window_win32::isVisible() const {
	return IsWindowVisible(hWnd);
}

void Window_win32::show(WindowShowCommand command) {
	int showCmd = SW_SHOW;
	switch (command) {
	case WindowShowCommand::HIDE:
		showCmd = SW_HIDE;
		break;
	case WindowShowCommand::SHOW:
		break;
	case WindowShowCommand::SHOW_MINIMIZED:
		showCmd = SW_SHOWMINIMIZED;
		break;
	case (WindowShowCommand::SHOW_RESTORED):
		showCmd = SW_SHOWNORMAL;
		break;
	case WindowShowCommand::SHOW_MAXIMIZED:
		showCmd = SW_SHOWMAXIMIZED;
		break;
	case WindowShowCommand::SHOW_FULL_SCREEN:
		showCmd = SW_SHOWMAXIMIZED;
		break;
	}
	auto style = GetWindowLongPtr(hWnd, GWL_STYLE);
	if (command == WindowShowCommand::SHOW_FULL_SCREEN && !(style & WS_POPUP)) {
		style |= WS_POPUP;
		style &= ~WS_OVERLAPPEDWINDOW;
		SetWindowLongPtr(hWnd, GWL_STYLE, style);
	} else if(style & WS_POPUP) {
		style &= ~WS_POPUP;
		style |= WS_OVERLAPPEDWINDOW;
		SetWindowLongPtr(hWnd, GWL_STYLE, style);
	}
	ShowWindow(hWnd, showCmd);
}

notifier::connection<void()> Window_win32::subscribeCloseEvent(
		std::function<void()>&& c) {
	return closeSubscribers.subscribe(std::move(c));
}

std::map<HWND, Window_win32*> Window_win32::hWndMap;

} /* namespace os */
} /* namespace dse */
