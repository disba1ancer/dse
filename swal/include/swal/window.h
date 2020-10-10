/*
 * window.h
 *
 *  Created on: 10 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_WINDOW_H_
#define WIN32_WINDOW_H_

#include <windows.h>
#include <string>
#include "error.h"
#include "enum_bitwise.h"
#include "zero_or_resource.h"

namespace swal {

enum class SetPosFlags {
	NoSize = SWP_NOSIZE,
	NoMove = SWP_NOMOVE,
	NoZOrder = SWP_NOZORDER,
	NoRedraw = SWP_NOREDRAW,
	NoActivate = SWP_NOACTIVATE,
	FrameChanged = SWP_FRAMECHANGED,
	ShowWindow = SWP_SHOWWINDOW,
	HideWindow = SWP_HIDEWINDOW,
	NoCopyBits = SWP_NOCOPYBITS,
	NoOwnerZOrder = SWP_NOOWNERZORDER,
	NoSendChanging = SWP_NOSENDCHANGING,
	DrawFrame = SWP_DRAWFRAME,
	NoReposition = SWP_NOREPOSITION,
	DeferErase = SWP_DEFERERASE,
	AsyncWindowPos = SWP_ASYNCWINDOWPOS,
};

template <>
struct enable_enum_bitwise<SetPosFlags> {
	static constexpr bool value = true;
};

enum class ShowCmd {
	Hide = SW_HIDE,
	ShowNormal = SW_SHOWNORMAL,
	Normal = SW_NORMAL,
	ShowMinimized = SW_SHOWMINIMIZED,
	ShowMaximized = SW_SHOWMAXIMIZED,
	Maximize = SW_MAXIMIZE,
	NoActivate = SW_SHOWNOACTIVATE,
	Show = SW_SHOW,
	Minimize = SW_MINIMIZE,
	ShowMinNoActive = SW_SHOWMINNOACTIVE,
	ShowNA = SW_SHOWNA,
	Restore = SW_RESTORE,
	ShowDefault = SW_SHOWDEFAULT,
	ForceMinimize = SW_FORCEMINIMIZE,
};

class Wnd : public zero_or_resource<HWND> {
public:
	Wnd(HWND hWnd) : zero_or_resource(hWnd) {}
	LONG_PTR GetLongPtr(int index) const {
		SetLastError(ERROR_SUCCESS);
		return error::throw_or_result(GetWindowLongPtr(*this, index), GetWindowLongPtr_error_check);
	}
	LONG_PTR SetLongPtr(int index, LONG_PTR value) const {
		SetLastError(ERROR_SUCCESS);
		return error::throw_or_result(SetWindowLongPtr(*this, index, value), GetWindowLongPtr_error_check);
	}
	void SetPos(const Wnd& wndAfter, int x, int y, int cx, int cy, SetPosFlags flags) const {
		error::throw_or_result(SetWindowPos(*this, wndAfter, x, y, cx, cy, static_cast<UINT>(flags)));
	}
	RECT GetRect() const {
		RECT rc;
		error::throw_or_result(GetWindowRect(*this, &rc));
		return rc;
	}
	RECT GetClientRect() const {
		RECT rc;
		error::throw_or_result(::GetClientRect(*this, &rc));
		return rc;
	}
	bool Show(ShowCmd cmd) const {
		return ShowWindow(*this, static_cast<int>(cmd));
	}
	void InvalidateRect(bool erase = true) const {
		error::throw_or_result(::InvalidateRect(*this, nullptr, erase));
	}
	void InvalidateRect(const RECT& rect, bool erase = true) const {
		error::throw_or_result(::InvalidateRect(*this, &rect, erase));
	}
	void ValidateRect() const {
		error::throw_or_result(::ValidateRect(*this, nullptr));
	}
	void ValidateRect(const RECT& rect) const {
		error::throw_or_result(::ValidateRect(*this, &rect));
	}
	bool IsVisible() const {
		return IsWindowVisible(*this);
	}
};

class Window : public Wnd {
public:
	Window(DWORD exStyle, ATOM cls, tstring_view wndName, DWORD style, int x, int y, int width, int height, const Wnd& parent, HMENU menu, HINSTANCE hInstance, void* param) :
		Wnd(swal::error::throw_or_result(CreateWindowEx(exStyle, reinterpret_cast<LPCTSTR>(cls), wndName.data(), style, x, y, width, height, parent, menu, hInstance, param)))
	{}
	Window(DWORD exStyle, ATOM cls, DWORD style, int x, int y, int width, int height, const Wnd& parent, HMENU menu, HINSTANCE hInstance, void* param) :
		Window(exStyle, cls, tstring_view(), style, x, y, width, height, parent, menu, hInstance, param)
	{}
	Window(ATOM cls, HINSTANCE hInstance, void* param = nullptr) :
		Window(0, cls, tstring_view(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, param)
	{}
	~Window() { DestroyWindow(*this); }
	Window(Window&&) = default;
	Window& operator=(Window&&) = default;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};

template <typename Cls, LRESULT(Cls::*mth)(HWND, UINT, WPARAM, LPARAM) = nullptr, int clsPtrIdx = GWLP_USERDATA>
LRESULT CALLBACK ClsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Cls* obj;
	Wnd wnd(hWnd);
	if (message == WM_NCCREATE) {
		auto cr = reinterpret_cast<CREATESTRUCT*>(lParam);
		obj = static_cast<Cls*>(cr->lpCreateParams);
		wnd.SetLongPtr(clsPtrIdx, reinterpret_cast<LONG_PTR>(obj));
	} else {
		obj = reinterpret_cast<Cls*>(wnd.GetLongPtr(clsPtrIdx));
	}
	if (obj) {
		if constexpr (mth) {
			return (obj->*mth)(hWnd, message, wParam, lParam);
		} else {
			return (*obj)(hWnd, message, wParam, lParam);
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

}

#endif /* WIN32_WINDOW_H_ */
