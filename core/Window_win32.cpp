/*
 * Window_win32.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "Window_win32.h"
#include <exception>
#include <dse/core/WindowEventData_win32.h>
#include <dse/core/PaintEventData_win32.h>
#include "errors_win32.h"

namespace dse::core {

Window_win32::Window_win32() try :
    wnd(makeWindowClsID(), static_cast<HINSTANCE>(GetModuleHandle(0)), this)
{} catch (std::system_error& e) {
    if (e.code().category() == swal::win32_category::instance()) {
        throw std::system_error(core::win32_errc(e.code().value()));
    } else {
        throw;
    }
}

LRESULT Window_win32::wndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) noexcept
{
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
		case WM_MOUSEMOVE: return onMouseMove(d);
		case WM_ACTIVATE: {
			if (!LOWORD(wParam)) {
				auto style = GetWindowLongPtr(hWnd, GWL_STYLE);
				if (style & WS_POPUP) {
					wnd.Show(swal::ShowCmd::Minimize);
				}
			}
			break;
		}
		/*case WM_SETCURSOR:
			if (LOWORD(lParam) == HTCLIENT) {
				SetCursor(NULL);
				return TRUE;
			}
			[[fallthrough]];*/
		default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM Window_win32::makeWindowClsID() {
	static ATOM clsID = []{
		HINSTANCE hInst = GetModuleHandle(nullptr);
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = swal::ClsWndProc<Window_win32, &Window_win32::wndProc, GwlpThis>;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(100));
		wcex.hCursor = LoadCursor(0, static_cast<LPCTSTR>(IDC_ARROW));
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = TEXT("dse.os.Window");
		wcex.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(100));

		return swal::winapi_call(RegisterClassEx(&wcex));
	}();
	return clsID;
}

Window_win32::~Window_win32()
{}

bool Window_win32::isVisible() const {
	return wnd.IsVisible();
}

void Window_win32::show(WindowShowCommand command) {
	using swal::ShowCmd;
	typedef swal::SetPosFlags SP;

	auto showCmd = ShowCmd::Show;
	switch (command) {
		case WindowShowCommand::Hide:
			showCmd = ShowCmd::Hide;
			break;
		case WindowShowCommand::Show:
			break;
		case WindowShowCommand::ShowMinimized:
			showCmd = ShowCmd::ShowMinimized;
			break;
		case (WindowShowCommand::ShowRestored):
			showCmd = ShowCmd::ShowNormal;
			break;
		case WindowShowCommand::ShowMaximized:
			showCmd = ShowCmd::ShowMaximized;
			break;
		case WindowShowCommand::ShowFullScreen:
			showCmd = ShowCmd::ShowMaximized;
			break;
	}
	auto style = wnd.GetLongPtr(GWL_STYLE);
	if (command == WindowShowCommand::ShowFullScreen && (style & WS_OVERLAPPEDWINDOW)) {
//		style |= WS_POPUP;
		style &= ~WS_OVERLAPPEDWINDOW;
		wnd.SetLongPtr(GWL_STYLE, style);
		wnd.SetPos(HWND_TOPMOST, 0, 0, 0, 0, SP::NoMove | SP::NoSize | SP::FrameChanged);
	} else if(!(style & WS_OVERLAPPEDWINDOW)) {
//		style &= ~WS_POPUP;
		style |= WS_OVERLAPPEDWINDOW;
		wnd.SetLongPtr(GWL_STYLE, style);
		wnd.SetPos(HWND_TOPMOST, 0, 0, 0, 0, SP::NoMove | SP::NoSize | SP::NoZOrder | SP::FrameChanged);
	}
	wnd.Show(showCmd);
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
	paintSubscribers.notify(d);
	return 0;
}

LRESULT Window_win32::onClose(WindowEventData_win32& d) {
	closeSubscribers.notify(d);
	return 0;
}

LRESULT Window_win32::onSize(WindowEventData_win32& d) {
	typedef WindowShowCommand WSC;
	WSC cmd = WSC::ShowRestored;

	switch (d.wParam) {
	case SIZE_MINIMIZED:
		cmd = WSC::ShowMinimized;
		break;
	case SIZE_MAXIMIZED:
		cmd = WSC::ShowMaximized;
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
	return *reinterpret_cast<const WindowData*>(wnd.get_ptr());
}

LRESULT Window_win32::onMouseMove(WindowEventData_win32 &d) {
	mouseMoveSubscribers.notify(d, GET_X_LPARAM(d.lParam), GET_Y_LPARAM(d.lParam));
	return 0;
}

notifier::connection<Window::MouseMoveHandler> Window_win32::subscribeMouseMoveEvent(
		std::function<Window::MouseMoveHandler> &&c) {
	return mouseMoveSubscribers.subscribe(std::move(c));
}

math::ivec2 Window_win32::size() {
	RECT rc = wnd.GetClientRect();
	return {rc.right, rc.bottom};
}

void Window_win32::resize(const math::ivec2& size) {
	typedef swal::SetPosFlags SP;
	RECT rc = { 0, 0, size[0], size[1] };
	DWORD style = wnd.GetLongPtr(GWL_STYLE);
	DWORD styleex = wnd.GetLongPtr(GWL_EXSTYLE);
	AdjustWindowRectEx(&rc, style, FALSE, styleex);
	wnd.SetPos(NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SP::NoActivate | SP::NoMove | SP::NoOwnerZOrder | SP::NoZOrder);
}

} /* namespace dse::core */
