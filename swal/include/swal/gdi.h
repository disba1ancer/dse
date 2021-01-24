/*
 * gdi.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_GDI_H
#define SWAL_GDI_H

#include "win_headers.h"
#include "error.h"
#include "zero_or_resource.h"
#include "enum_bitwise.h"

namespace swal {

class GdiObj : public zero_or_resource<HGDIOBJ> {
public:
	GdiObj(HGDIOBJ obj) : zero_or_resource(obj) {}
	~GdiObj() {
		DeleteObject(*this);
	}
	GdiObj(const GdiObj&) = delete;
	GdiObj& operator=(const GdiObj&) = delete;
	GdiObj(GdiObj&&) = default;
	GdiObj& operator=(GdiObj&&) = default;
};

enum class PenStyle {
	Solid = PS_SOLID,
	Dash = PS_DASH,
	Dot = PS_DOT,
	DashDot = PS_DASHDOT,
	DashDotDot = PS_DASHDOTDOT,
	Null = PS_NULL,
	InsideFrame = PS_INSIDEFRAME
};

inline constexpr COLORREF Rgb(int r, int g, int b) { return RGB(r, g, b); }

class Pen : public GdiObj {
public:
	Pen(COLORREF color, PenStyle style = PenStyle::Solid, int width = 1) : GdiObj(winapi_call(CreatePen(static_cast<int>(style), width, color))) {}
};

class DC {
public:
	DC(HDC hdc) : hdc(hdc) {}
	operator HDC() const { return hdc; }
	HGDIOBJ SelectObject(HGDIOBJ obj) const { return winapi_call(::SelectObject(hdc, obj)); }
	void MoveTo(int x, int y) const { winapi_call(::MoveToEx(hdc, x, y, nullptr)); }
	POINT MoveToEx(int x, int y) const {
		POINT pt;
		winapi_call(::MoveToEx(hdc, x, y, &pt));
		return pt;
	}
	void LineTo(int x, int y) const { winapi_call(::LineTo(hdc, x, y)); }
	COLORREF SetPenColor(COLORREF color) const { return winapi_call(::SetDCPenColor(*this, color), invalid_color_error_check); }
	COLORREF SetPixel(int x, int y, COLORREF color) const { return winapi_call(::SetPixel(*this, x, y, color), invalid_color_error_check); }
	void FillRect(RECT& rc, HBRUSH brush) const { winapi_call(::FillRect(*this, &rc, brush)); }
private:
	HDC hdc;
};

class PaintDC : private PAINTSTRUCT, public DC {
public:
	PaintDC(HWND hWnd) : DC(winapi_call(::BeginPaint(hWnd, this))), hWnd(hWnd) {}
	~PaintDC() { EndPaint(hWnd, this); }
	PaintDC(const PaintDC&) = delete;
	PaintDC& operator=(const PaintDC&) = delete;
	PaintDC(PaintDC&&) = default;
	PaintDC& operator=(PaintDC&&) = default;
	const PAINTSTRUCT* operator ->() const { return this; }
	//static PaintDC BeginPaint(HWND hWnd) { return PaintDC(hWnd); }
private:
	HWND hWnd;
};

enum class GetDCExFlags {
	Window = DCX_WINDOW,
	Cache = DCX_CACHE,
	NoResetAttrs = DCX_NORESETATTRS,
	ClipChildren = DCX_CLIPCHILDREN,
	ClipSiblings = DCX_CLIPSIBLINGS,
	ParentClip = DCX_PARENTCLIP,
	ExcludeRGN = DCX_EXCLUDERGN,
	IntersectRGN = DCX_INTERSECTRGN,
	ExcludeUpdate = DCX_EXCLUDEUPDATE,
	IntersectUpdate = DCX_INTERSECTUPDATE,
	LockUindowUpdate = DCX_LOCKWINDOWUPDATE
};

template <> struct enable_enum_bitwise<GetDCExFlags> : std::true_type {};

class WindowDC : public DC {
public:
	WindowDC(HWND hWnd) : DC(winapi_call(GetDC(hWnd))) {}
	WindowDC(HWND hWnd, HRGN clip, DWORD flags) : DC(winapi_call(GetDCEx(hWnd, clip, flags))) {}
	WindowDC(HWND hWnd, HRGN clip, GetDCExFlags flags) : WindowDC(hWnd, clip, DWORD(flags)) {}
	~WindowDC() { ReleaseDC(hWnd, *this); }
	WindowDC(const WindowDC&) = delete;
	WindowDC& operator=(const WindowDC&) = delete;
	WindowDC(WindowDC&&) = default;
	WindowDC& operator=(WindowDC&&) = default;
private:
	HWND hWnd;
};

}

#endif /* SWAL_GDI_H */
