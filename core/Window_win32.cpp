/*
 * Window_win32.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "Window_win32.h"
#include <exception>
#include <dse/core/Window_win32.h>
#include "errors_win32.h"
#include "SystemLoop_win32.h"
#include <swal/hinstance.h>
#include <dse/math/vmath.h>

namespace dse::core {

Window_win32::Window_win32(SystemLoop& loop) try :
    loop(&loop)
{
    wnd.Create(
        0, WindowClass(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        NULL, NULL,
        swal::GetLocalInstance(), this
    );
} catch (std::system_error& e) {
    if (e.code().category() == swal::win32_category::instance()) {
        throw std::system_error(core::win32_errc(e.code().value()));
    } else {
        throw;
    }
}

Window_win32::~Window_win32()
{}

bool Window_win32::IsVisible() const {
	return visible;
}

bool Window_win32::IsFullscreen() const
{
	return fullscreen;
}

void Window_win32::Show(WindowShowCommand command)
{
	using enum WindowShowCommand;
	using SC = swal::ShowCmd;
	using enum swal::SetPosFlags;

	if (fullscreen) {
		if (command == ShowFullScreen) {
			return;
		}
		fullscreen = false;
		auto [exStyle, style] = CalcStyles();
		wnd.SetLongPtr(GWL_STYLE, style ^ WS_VISIBLE & style);
		wnd.SetLongPtr(GWL_EXSTYLE, exStyle);
		swal::winapi_call(::SetWindowPlacement(wnd, &normPlace));
		if (command == ShowRestored) {
			return;
		}
	}

	auto showCmd = SC::Show;
	switch (command) {
	case Hide:
		showCmd = SC::Hide;
		break;
	case Show:
		break;
	case ShowMinimized:
		showCmd = SC::ShowMinimized;
		break;
	case ShowNormal:
		showCmd = SC::ShowNormal;
		break;
	case ShowMaximized:
		showCmd = SC::ShowMaximized;
		break;
	case ShowRestored:
		showCmd = SC::Restore;
		break;
	case ShowFullScreen: {
		auto mon = ::MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
		math::ivec2 pos, size;
		{
			MONITORINFO minfo = {sizeof(minfo)};
			swal::winapi_call(::GetMonitorInfo(mon, &minfo));
			pos = {minfo.rcMonitor.left, minfo.rcMonitor.top};
			size = {minfo.rcMonitor.right, minfo.rcMonitor.bottom};
			size -= pos;
		}
		swal::winapi_call(::GetWindowPlacement(wnd, &normPlace));
		fullscreen = true;
		auto [exStyle, style] = CalcStyles();
		wnd.SetLongPtr(GWL_STYLE, style ^ WS_VISIBLE & style);
		wnd.SetLongPtr(GWL_EXSTYLE, exStyle);
		wnd.SetPos(HWND_TOP, pos.x(), pos.y(), size.x(), size.y(), FrameChanged | ShowWindow | NoRedraw);
		swal::winapi_call(::RedrawWindow(wnd, nullptr, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE | RDW_ERASENOW));
		return;
	}
	}
	wnd.Show(showCmd);
}

auto Window_win32::GetSysData()
-> const WindowData&
{
	return *reinterpret_cast<const WindowData*>(wnd.get_ptr());
}

auto Window_win32::Position() const -> math::ivec2
{
	return pos;
}

void Window_win32::Move(const math::ivec2& pos)
{
	typedef swal::SetPosFlags SP;
	wnd.SetPos(NULL, pos.x(), pos.y(), 0, 0, SP::NoActivate | SP::NoSize | SP::NoOwnerZOrder | SP::NoZOrder);
}

auto Window_win32::Size() const -> math::ivec2
{
	return size;
}

void Window_win32::Resize(const math::ivec2& size)
{
	typedef swal::SetPosFlags SP;
	wnd.SetPos(NULL, 0, 0, size.x(), size.y(), SP::NoActivate | SP::NoMove | SP::NoOwnerZOrder | SP::NoZOrder | SP::FrameChanged);
}

auto Window_win32::SurfaceSize() const -> math::ivec2
{
	return clientSize;
}

void Window_win32::ResizeSurface(const math::ivec2& size)
{
	typedef swal::SetPosFlags SP;
	RECT rc = { 0, 0, size[0], size[1] };
	DWORD style = wnd.GetLongPtr(GWL_STYLE);
	DWORD exStyle = wnd.GetLongPtr(GWL_EXSTYLE);
	::AdjustWindowRectEx(&rc, style, FALSE, exStyle);
	wnd.SetPos(NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SP::NoActivate | SP::NoMove | SP::NoOwnerZOrder | SP::NoZOrder);
}

void Window_win32::SetTitle(const char8_t* title)
{
	auto wtitle = swal::u8_to_wide_char(title);
	swal::winapi_call(::SetWindowText(wnd, wtitle.c_str()));
}

void Window_win32::ChangeFrameStyle(WindowFrameStyle style)
{
	frameStyle = style;
	ApplyStyles();
}

bool Window_win32::Minimizable() const
{
	return minimizable;
}

void Window_win32::MakeMinimizable(bool state)
{
	minimizable = state;
	ApplyStyles();
}

bool Window_win32::HasMaximizeCtl() const
{
	return maximizable;
}

void Window_win32::ShowMaximizeCtl(bool state)
{
	maximizable = state;
	ApplyStyles();
}

auto Window_win32::GetLoop() const -> SystemLoop&
{
	return *loop;
}

bool Window_win32::Register(WindowEvent evt, void* object, void (*cb)())
{
	return eventmgr.register_e(evt, object, cb);
}

void Window_win32::Unregister(WindowEvent evt, void* object, void (*cb)()) noexcept
{
	eventmgr.unregister(evt, object, cb);
}

void Window_win32::fill_class_info(WNDCLASSEX& wcex)
{
	wcex.style |= CS_OWNDC;
	wcex.hIcon = NULL;
	wcex.hIconSm = NULL;
}

auto Window_win32::WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) noexcept -> LRESULT
{
	switch(message) {
	HANDLE_MSG(hWnd, WM_NCCREATE, OnNCCreate);
	HANDLE_MSG(hWnd, WM_NCCALCSIZE, OnNCCalcSize);
	HANDLE_MSG(hWnd, WM_CLOSE, OnClose);
	HANDLE_MSG(hWnd, WM_ERASEBKGND, OnEraseBkgnd);
	HANDLE_MSG(hWnd, WM_KEYDOWN, OnKey);
	HANDLE_MSG(hWnd, WM_KEYUP, OnKey);
	HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);
	HANDLE_MSG(hWnd, WM_WINDOWPOSCHANGING, OnWindowPosChanging);
	HANDLE_MSG(hWnd, WM_WINDOWPOSCHANGED, OnWindowPosChanged);
	HANDLE_MSG(hWnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
	}
	return ForwardMsg(hWnd, message, wParam, lParam);
}

auto Window_win32::WindowClass() -> LPCTSTR {
	static swal::auto_window_class<
		Window_win32,
		&Window_win32::wnd,
		&Window_win32::WndProc
	> cls;
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
	using enum WindowShowCommand;
	WinStyles r;
	r.style = WS_SYSMENU;
	r.exStyle = 0;
	r.style |= WS_VISIBLE * visible;
	if (minimizable) {
		r.style |= WS_MINIMIZEBOX;
	}
	if (fullscreen) {
		return r;
	}
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
	if (state == ShowMinimized) {
		r.style |= WS_MINIMIZE | (WS_MAXIMIZE * maximize);
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

BOOL Window_win32::OnNCCreate(HWND hwnd, CREATESTRUCT* lpCreateStruct)
{
	return FORWARD_WM_NCCREATE(hwnd, lpCreateStruct, DefWindowProc);
}

auto Window_win32::OnNCCalcSize(HWND hwnd, BOOL fCalcValidRects, NCCALCSIZE_PARAMS* lpcsp) -> UINT
{
	auto& rc = fCalcValidRects ? lpcsp->rgrc[0] : *reinterpret_cast<RECT*>(lpcsp);
	size = { rc.right - rc.left, rc.bottom - rc.top };
	auto result = FORWARD_WM_NCCALCSIZE(hwnd, fCalcValidRects, lpcsp, DefWindowProc);
	clientSize = { rc.right - rc.left, rc.bottom - rc.top };
	return result;
}

void Window_win32::OnClose(HWND hwnd)
{
	eventmgr.send<WindowEvent::Close>();
}

BOOL Window_win32::OnEraseBkgnd(HWND hwnd, HDC hdc)
{
	using enum WindowEvent;
	bool result = false;
	if (!eventmgr.send<System + WM_ERASEBKGND>(hwnd, hdc, result)) {
		return FORWARD_WM_ERASEBKGND(hwnd, hdc, DefWindowProc);
	}
	return result;
}

void Window_win32::OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	KeyboardKeyState state = (fDown ? (flags & KF_REPEAT ?
		KeyboardKeyState::PRESSED :
		KeyboardKeyState::DOWN) :
		KeyboardKeyState::UP);
	eventmgr.send<WindowEvent::Key>(state, vk);
}

void Window_win32::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	eventmgr.send<WindowEvent::MouseMove>(x, y);
}

namespace {

auto IsMoving(const WINDOWPOS* wPos) -> bool
{
    return (wPos->flags & SWP_NOMOVE) == 0;
}

auto IsSizing(const WINDOWPOS* wPos) -> bool
{
    return (wPos->flags & SWP_NOSIZE) == 0;
}

auto IsFrameChanging(const WINDOWPOS* wPos) -> bool
{
    return wPos->flags & SWP_FRAMECHANGED;
}

auto IsShowing(const WINDOWPOS* wPos) -> bool
{
    return wPos->flags & SWP_SHOWWINDOW;
}

auto IsHiding(const WINDOWPOS* wPos) -> bool
{
    return wPos->flags & SWP_HIDEWINDOW;
}

}

BOOL Window_win32::OnWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos)
{
    return FORWARD_WM_WINDOWPOSCHANGING(hwnd, lpwpos, DefWindowProc);
}

void Window_win32::OnWindowPosChanged(HWND hwnd, const WINDOWPOS* lpwpos)
{
    if (IsMoving(lpwpos)) {
        pos = {lpwpos->x, lpwpos->y};
    }
    if (IsSizing(lpwpos) || IsFrameChanging(lpwpos)) {
        eventmgr.send<WindowEvent::Resize>();
    }
    if (IsShowing(lpwpos)) {
        visible = true;
    }
    if (IsHiding(lpwpos)) {
        visible = false;
    }
    if (IsFrameChanging(lpwpos)) {
        auto style = wnd.GetLongPtr(GWL_STYLE);
        maximize = style & WS_MAXIMIZE;
        using enum WindowShowCommand;
        if (style & WS_MINIMIZE) {
            state = ShowMinimized;
        } else if (maximize) {
            state = ShowMaximized;
        } else {
            state = ShowNormal;
        }
    }
}

void Window_win32::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	if (minSize.x() >= 0) {
		lpMinMaxInfo->ptMinTrackSize = {minSize.x(), minSize.y()};
	}
	if (maxSize.x() >= 0) {
		lpMinMaxInfo->ptMaxTrackSize = {maxSize.x(), maxSize.y()};
	}
}

LRESULT Window_win32::ForwardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using handler_t = typename util::event_traits<WindowEvent::System>::handler;
	auto event = WindowEvent::System + message;
	if (eventmgr.send<handler_t>(event, hWnd, wParam, lParam)) {
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

} /* namespace dse::core */
