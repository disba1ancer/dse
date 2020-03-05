/*
 * Window_win32.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "Window_win32.h"
#include <exception>
#include "WindowEventData_win32.h"
#include "PaintEventData_win32.h"

namespace dse {
namespace os {

Window_win32::Window_win32() : hWnd(CreateWindowEx(0, reinterpret_cast<LPCTSTR>(makeWindowClsID()), 0,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		0, 0, static_cast<HINSTANCE>(GetModuleHandle(0)), this)) {
	if (hWnd == 0) {
		throw std::runtime_error("Fail to create window");
	}
}

LRESULT CALLBACK Window_win32::staticWndProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam) {
	Window_win32 *aThis;
	if (message == WM_NCCREATE){
		aThis = reinterpret_cast<Window_win32*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_THIS, reinterpret_cast<LONG_PTR>(aThis));
	} else {
		aThis = reinterpret_cast<Window_win32*>(GetWindowLongPtr(hWnd, GWLP_THIS));
		if (!aThis) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return aThis->wndProc(hWnd, message, wParam, lParam);
}

LRESULT Window_win32::wndProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam) {
	WindowEventData_win32 d{hWnd, message, wParam, lParam};
	switch(message) {
	case WM_PAINT: return onPaint(d);
	case WM_CLOSE: return onClose(d);
	case WM_SIZE: return onSize(d);
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN: return onKeyDown(d);
	case WM_SYSKEYUP:
	case WM_KEYUP: return onKeyUp(d);
	case WM_SYSCHAR: break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
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
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = static_cast<HINSTANCE>(GetModuleHandle(0));
		wcex.hIcon = LoadIcon(0, static_cast<LPCTSTR>(IDI_APPLICATION));
		wcex.hCursor = LoadCursor(0, static_cast<LPCTSTR>(IDC_ARROW));
		wcex.hbrBackground = NULL;
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

notifier::connection<Window::CloseHandler> Window_win32::subscribeCloseEvent(
		std::function<Window::CloseHandler>&& c) {
	return closeSubscribers.subscribe(std::move(c));
}

notifier::connection<Window::ResizeHandler> Window_win32::subscribeResizeEvent(
		std::function<Window::ResizeHandler>&& c) {
	return resizeSubscribers.subscribe(std::move(c));
}

LRESULT Window_win32::onPaint(WindowEventData_win32& d) {
	PAINTSTRUCT ps;
	PaintEventData_win32 pd = { BeginPaint(hWnd, &ps) };
	paintSubscribers.notify(d, pd);
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT Window_win32::onClose(WindowEventData_win32& d) {
	closeSubscribers.notify(d);
	return 0;
}

LRESULT Window_win32::onSize(WindowEventData_win32& d) {
	typedef WindowShowCommand WSC;
	WSC cmd = WSC::SHOW_RESTORED;

	switch (d.wParam) {
	case SIZE_MINIMIZED:
		cmd = WSC::SHOW_MINIMIZED;
		break;
	case SIZE_MAXIMIZED:
		cmd = WSC::SHOW_MAXIMIZED;
		break;
	}

	resizeSubscribers.notify(d, LOWORD(d.lParam), HIWORD(d.lParam), cmd);
	return 0;
}

LRESULT Window_win32::onKeyDown(WindowEventData_win32& d) {
	KeyboardKeyState state = (d.lParam & (1 << 30)
			? KeyboardKeyState::PRESSED : KeyboardKeyState::DOWN);
	keySubscribers.notify(d, state, d.wParam);
	return 0;
}

LRESULT Window_win32::onKeyUp(WindowEventData_win32& d) {
	keySubscribers.notify(d, KeyboardKeyState::UP, d.wParam);
	return 0;
}

notifier::connection<Window::KeyHandler> Window_win32::subscribeKeyEvent(
		std::function<Window::KeyHandler> &&c) {
	return keySubscribers.subscribe(std::move(c));
}

notifier::connection<Window::PaintHandler> Window_win32::subscribePaintEvent(
		std::function<Window::PaintHandler> &&c) {
	return paintSubscribers.subscribe(std::move(c));
}

const WindowData& Window_win32::getSysData() {
	return *reinterpret_cast<const WindowData*>(&hWnd);
}

} /* namespace os */
} /* namespace dse */
