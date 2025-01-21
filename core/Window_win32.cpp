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
    swal::Wnd owner = SystemLoop_win32::GetImpl(loop)->OwnerWindow();
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
	WindowEventData_win32 d{hWnd, message, wParam, lParam};
	switch(message) {
	case WM_NCCREATE: return OnNCCreate(d);
	case WM_NCCALCSIZE: return OnNCCalcSize(d);
	case WM_CLOSE: return OnClose(d);
	case WM_ERASEBKGND: return OnErase(d);
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
	default: return FwdMessage(d);
	}
	return 0;
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

auto Window_win32::OnClose(WindowEventData_win32& d) -> LRESULT
{
	eventmgr.send<WindowEvent::Close>();
	return 0;
}

auto Window_win32::OnKeyDown(WindowEventData_win32& d) -> LRESULT
{
	KeyboardKeyState state = (d.lParam & (1 << 30)
			? KeyboardKeyState::PRESSED : KeyboardKeyState::DOWN);
	eventmgr.send<WindowEvent::Key>(state, d.wParam);
	return 0;
}

auto Window_win32::OnKeyUp(WindowEventData_win32& d) -> LRESULT
{
	eventmgr.send<WindowEvent::Key>(KeyboardKeyState::UP, d.wParam);
	return 0;
}

auto Window_win32::OnMouseMove(WindowEventData_win32 &d) -> LRESULT
{
	eventmgr.send<WindowEvent::MouseMove>(GET_X_LPARAM(d.lParam), GET_Y_LPARAM(d.lParam));
    return 0;
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

auto Window_win32::OnPosChanging(WindowEventData_win32& d) -> LRESULT
{
    return CallDefWindowProc(d);
}

auto Window_win32::OnPosChanged(WindowEventData_win32& d) -> LRESULT
{
    auto wPos = reinterpret_cast<const WINDOWPOS*>(d.lParam);
    if (IsMoving(wPos)) {
        pos = {wPos->x, wPos->y};
    }
    if (IsSizing(wPos) || IsFrameChanging(wPos)) {
        eventmgr.send<WindowEvent::Resize>();
    }
    if (IsShowing(wPos)) {
        visible = true;
    }
    if (IsHiding(wPos)) {
        visible = false;
    }
    if (IsFrameChanging(wPos)) {
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
    return 0;
}

auto Window_win32::OnNCCreate(WindowEventData_win32& d) -> LRESULT
{
	auto cr = reinterpret_cast<CREATESTRUCT*>(d.lParam);
	if (cr->x == CW_USEDEFAULT || cr->cx == CW_USEDEFAULT) {
		auto&& rc = wnd.GetRect();
		pos = {rc.left, rc.top};
		size = math::ivec2{rc.right, rc.bottom} - pos;
	} else {
		pos = {cr->x, cr->y};
		size = {cr->cx, cr->cy};
	}
	return DefWindowProc(d.hWnd, d.message, d.wParam, d.lParam);
}

LRESULT Window_win32::OnNCCalcSize(WindowEventData_win32& d)
{
	auto& rc = [&d] -> auto&
	{
		if (d.wParam == FALSE) {
			return *reinterpret_cast<RECT*>(d.lParam);
		}
		return reinterpret_cast<NCCALCSIZE_PARAMS*>(d.lParam)->rgrc[0];
	}();
	size = { rc.right - rc.left, rc.bottom - rc.top };
	auto result = CallDefWindowProc(d);
	clientSize = { rc.right - rc.left, rc.bottom - rc.top };
	return result;
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

LRESULT Window_win32::OnErase(WindowEventData_win32& d)
{
	using enum WindowEvent;
	LRESULT result = FALSE;
	auto hdc = reinterpret_cast<HDC>(d.wParam);
	if (!eventmgr.send<System + WM_ERASEBKGND>(d.hWnd, hdc, result)) {
		result = CallDefWindowProc(d);
	}
	return result;
}

auto Window_win32::CallDefWindowProc(WindowEventData_win32& d) -> LRESULT
{
	return DefWindowProc(d.hWnd, d.message, d.wParam, d.lParam);
}

LRESULT Window_win32::FwdMessage(WindowEventData_win32& d)
{
	using handler_t = typename util::event_traits<WindowEvent::System>::handler;
	auto event = WindowEvent::System + d.message;
	if (eventmgr.send<handler_t>(event, d.hWnd, d.wParam, d.lParam)) {
		return 0;
	}
	return CallDefWindowProc(d);
}

void Window_win32::SetTitle(const char8_t* title)
{
	auto wtitle = swal::u8_to_wide_char(title);
	swal::winapi_call(::SetWindowText(wnd, wtitle.c_str()));
}

} /* namespace dse::core */
