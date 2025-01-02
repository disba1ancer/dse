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
#include "SystemLoop_win32.h"
#include <swal/hinstance.h>
#include <dse/math/vmath.h>
#include <format>
#include <iostream>
#include <dwmapi.h>
#include <Uxtheme.h>

namespace dse::core {

Window_win32::Window_win32(SystemLoop& loop) try :
    loop(&loop)
{
    swal::Wnd owner = SystemLoop_win32::GetImpl(loop)->OwnerWindow();
    wnd.Create(
        WS_EX_APPWINDOW, WindowClass(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        owner, NULL,
        swal::GetLocalInstance(), this
    );
    MARGINS m = { -1, -1, -1, -1 };
    ::DwmExtendFrameIntoClientArea(wnd, &m);
    // owner.Show(swal::ShowCmd::Show);
} catch (std::system_error& e) {
    if (e.code().category() == swal::win32_category::instance()) {
        throw std::system_error(core::win32_errc(e.code().value()));
    } else {
        throw;
    }
}

auto Window_win32::WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) noexcept -> LRESULT
{
	WindowEventData_win32 d{hWnd, message, wParam, lParam};
	switch(message) {
	case WM_NCCREATE: return OnNCCreate(d);
	case WM_ERASEBKGND: return TRUE;
	case WM_PAINT: return OnPaint(d);
	case WM_CLOSE: return OnClose(d);
	case WM_SIZE: return OnSize(d);
	// case WM_SYSKEYDOWN:
	case WM_KEYDOWN: return OnKeyDown(d);
	// case WM_SYSKEYUP:
	case WM_KEYUP: return OnKeyUp(d);
	case WM_SYSCHAR: break;
	case WM_MOUSEMOVE: return OnMouseMove(d);
	case WM_WINDOWPOSCHANGING: return OnPosChanging(d);
	case WM_WINDOWPOSCHANGED: return OnPosChanged(d);
	case WM_GETMINMAXINFO: return OnGetMinMaxInfo(d);
	/*case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT) {
			SetCursor(NULL);
			return TRUE;
		}
		[[fallthrough]];*/
	default: return CallDefWindowProc(d);
	}
	return 0;
}

auto Window_win32::WindowClass() -> LPCTSTR {
	static swal::auto_window_class<Window_win32, &Window_win32::WndProc> cls;
	return cls;
}

auto Window_win32::DefMinSize() -> math::ivec2
{
	return {
		::GetSystemMetrics(SM_CXMINTRACK),
		::GetSystemMetrics(SM_CYMINTRACK)
	};
}

auto Window_win32::DefMaxSize() -> math::ivec2
{
	return {
		::GetSystemMetrics(SM_CXMAXTRACK),
		::GetSystemMetrics(SM_CYMAXTRACK)
	};
}

auto Window_win32::MinSize() const -> math::ivec2
{
	if (minSize.x() < 0) {
		return DefMinSize();
	}
	return minSize;
}

auto Window_win32::MaxSize() const -> math::ivec2
{
	if (maxSize.x() < 0) {
		return DefMaxSize();
	}
	return maxSize;
}

auto Window_win32::ClampSize(const math::ivec2& size) const -> math::ivec2
{
	return max(MinSize(), min(size, MaxSize()));
}

auto Window_win32::CalcStyles() -> WinStyles
{
	using enum WindowFrameStyle;
	WinStyles r;
	r.style = WS_SYSMENU;
	r.exStyle = WS_EX_APPWINDOW;
	switch (frameStyle) {
	case None:
		break;
	case Fixed:
		r.style |= WS_CAPTION;
		break;
	case Sizable:
		r.style |= WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX;
		break;
	}
	using enum WindowFlags;
	if (+(flags & Minimizable)) {
		r.style |= WS_MINIMIZEBOX;
	}
	return r;
}

void Window_win32::ApplyStyles()
{
	auto [exStyle, style] = CalcStyles();
	wnd.SetLongPtr(GWL_EXSTYLE, exStyle);
	wnd.SetLongPtr(GWL_STYLE, style);
	using enum swal::SetPosFlags;
	wnd.SetPos(NULL, 0, 0, 0, 0, NoActivate | NoZOrder | NoMove | NoSize | FrameChanged);
}

Window_win32::~Window_win32()
{}

bool Window_win32::IsVisible() const {
	return wnd.IsVisible();
}

void Window_win32::Show(WindowShowCommand command) {
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
	// auto style = wnd.GetLongPtr(GWL_STYLE);
	// auto fullscreen = !(style & WS_OVERLAPPEDWINDOW);
	// if (command == WindowShowCommand::ShowFullScreen && !fullscreen) {
	// 	wnd.SetLongPtr(GWL_STYLE, style ^ (style & (WS_OVERLAPPEDWINDOW | WS_MAXIMIZE)));
	// 	// wnd.SetPos(HWND_TOPMOST, 0, 0, 0, 0, SP::NoMove | SP::NoSize | SP::FrameChanged);
	// } else if(command != WindowShowCommand::ShowFullScreen && fullscreen) {
	// 	wnd.SetLongPtr(GWL_STYLE, style | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE);
	// 	// wnd.SetPos(NULL, 0, 0, 0, 0, SP::NoMove | SP::NoSize | SP::NoZOrder | SP::FrameChanged);
	// }
	wnd.Show(showCmd);
	if (showCmd == ShowCmd::ShowMaximized && frameStyle == WindowFrameStyle::None) {
		auto menu = ::GetSystemMenu(wnd, FALSE);
		::EnableMenuItem(menu, SC_RESTORE, MF_DISABLED | MF_GRAYED);
	}
}

auto Window_win32::SubscribeCloseEvent(std::function<Window::CloseHandler>&& c)
-> notifier::connection<Window::CloseHandler>
{
	return closeSubscribers.subscribe(std::move(c));
}

auto Window_win32::SubscribeResizeEvent(
		std::function<Window::ResizeHandler>&& c)
-> notifier::connection<Window::ResizeHandler>
{
	return resizeSubscribers.subscribe(std::move(c));
}

auto Window_win32::OnPaint(WindowEventData_win32& d) -> LRESULT
{
	paintSubscribers.notify(d);
	return 0;
}

auto Window_win32::OnClose(WindowEventData_win32& d) -> LRESULT
{
	closeSubscribers.notify(d);
	return 0;
}

auto Window_win32::OnSize(WindowEventData_win32& d) -> LRESULT
{
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

auto Window_win32::OnKeyDown(WindowEventData_win32& d) -> LRESULT
{
	KeyboardKeyState state = (d.lParam & (1 << 30)
			? KeyboardKeyState::PRESSED : KeyboardKeyState::DOWN);
	keySubscribers.notify(d, state, d.wParam);
	return 0;
}

auto Window_win32::OnKeyUp(WindowEventData_win32& d) -> LRESULT
{
	keySubscribers.notify(d, KeyboardKeyState::UP, d.wParam);
	return 0;
}

auto Window_win32::SubscribeKeyEvent(
		std::function<Window::KeyHandler> &&c)
-> notifier::connection<Window::KeyHandler>
{
	return keySubscribers.subscribe(std::move(c));
}

auto Window_win32::SubscribePaintEvent(
		std::function<Window::PaintHandler> &&c)
-> notifier::connection<Window::PaintHandler>
{
	return paintSubscribers.subscribe(std::move(c));
}

auto Window_win32::GetSysData()
-> const WindowData&
{
	return *reinterpret_cast<const WindowData*>(wnd.get_ptr());
}

auto Window_win32::OnMouseMove(WindowEventData_win32 &d) -> LRESULT
{
	mouseMoveSubscribers.notify(d, GET_X_LPARAM(d.lParam), GET_Y_LPARAM(d.lParam));
    return 0;
}

auto Window_win32::OnPosChanging(WindowEventData_win32& d) -> LRESULT
{
    return CallDefWindowProc(d);
}

auto Window_win32::OnPosChanged(WindowEventData_win32& d) -> LRESULT
{
    auto wPos = reinterpret_cast<const WINDOWPOS*>(d.lParam);
    if ((wPos->flags & SWP_NOMOVE) == 0) {
        pos = {wPos->x, wPos->y};
    }
    if ((wPos->flags & SWP_NOSIZE) == 0) {
        size = {wPos->cx, wPos->cy};
    }
    return 0;
}

auto Window_win32::OnNCCreate(WindowEventData_win32& d) -> LRESULT
{
	wnd = d.hWnd;
	auto cr = reinterpret_cast<CREATESTRUCT*>(d.lParam);
	if (cr->x == CW_USEDEFAULT || cr->cx == CW_USEDEFAULT) {
		auto&& rc = wnd.GetRect();
		pos = {rc.left, rc.top};
		size = math::ivec2{rc.right, rc.bottom} - pos;
	} else {
		pos = {cr->x, cr->y};
		size = {cr->cx, cr->cy};
	}
	// swal::com_call(::SetWindowTheme(wnd, L" ", L" "));
	wnd.SetClassLongPtr(GCLP_HICON, NULL);
	wnd.SetClassLongPtr(GCLP_HICONSM, NULL);
	// auto v = DWMNCRP_DISABLED;
	// ::DwmSetWindowAttribute(wnd, DWMWA_NCRENDERING_POLICY, &v, sizeof(v));
	// wnd.SetLongPtr(GWL_STYLE, WS_CHILD);
	// wnd.SetLongPtr(GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_STATICEDGE);
	// using enum swal::SetPosFlags;
	// wnd.SetPos(0, 0, 0, 0, 0, NoZOrder | NoMove| NoSize | FrameChanged);
	return TRUE;
}

auto Window_win32::OnGetMinMaxInfo(WindowEventData_win32& d) -> LRESULT
{
	auto mmi = reinterpret_cast<MINMAXINFO*>(d.lParam);
	if (minSize.x() >= 0) {
		mmi->ptMinTrackSize = {minSize.x(), minSize.y()};
	}
	if (maxSize.x() >= 0) {
		mmi->ptMaxTrackSize = {maxSize.x(), maxSize.y()};
	}
	return 0;
}

auto Window_win32::CallDefWindowProc(WindowEventData_win32& d) -> LRESULT
{
	return DefWindowProc(d.hWnd, d.message, d.wParam, d.lParam);
}

auto Window_win32::SubscribeMouseMoveEvent(std::function<Window::MouseMoveHandler> &&c)
-> notifier::connection<Window::MouseMoveHandler>
{
	return mouseMoveSubscribers.subscribe(std::move(c));
}

auto Window_win32::Size() const -> math::ivec2 {
	return size;
}

void Window_win32::Resize(const math::ivec2& size) {
	typedef swal::SetPosFlags SP;
	RECT rc = { 0, 0, size[0], size[1] };
	DWORD style = wnd.GetLongPtr(GWL_STYLE);
	DWORD exStyle = wnd.GetLongPtr(GWL_EXSTYLE);
	AdjustWindowRectEx(&rc, style, FALSE, exStyle);
	wnd.SetPos(NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SP::NoActivate | SP::NoMove | SP::NoOwnerZOrder | SP::NoZOrder);
}

void Window_win32::ChangeFrameStyle(WindowFrameStyle style)
{
	frameStyle = style;
	ApplyStyles();
}

bool Window_win32::HasMinimizeCtl()
{
	return bool(flags & WindowFlags::Minimizable);
}

void Window_win32::ShowMinimizeCtl(bool state)
{
	constexpr auto flag = WindowFlags::Minimizable;
	flags = (flags | flag) ^ flag * !state;
	ApplyStyles();
}

bool Window_win32::HasMaximizeCtl()
{
	return bool(flags & WindowFlags::Maximizable);
}

void Window_win32::ShowMaximizeCtl(bool state)
{
	constexpr auto flag = WindowFlags::Maximizable;
	flags = (flags | flag) ^ flag * !state;
	ApplyStyles();
}

auto Window_win32::GetLoop() const -> SystemLoop&
{
	return *loop;
}

void Window_win32::SetTitle(const char8_t* title)
{
	auto wtitle = swal::u8_to_wide_char(title);
	swal::winapi_call(::SetWindowText(wnd, wtitle.c_str()));
}

} /* namespace dse::core */
