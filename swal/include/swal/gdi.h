/*
 * gdi.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_GDI_H_
#define WIN32_GDI_H_

#include <windows.h>
#include "error.h"
#include "zero_or_resource.h"

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

COLORREF Rgb(int r, int g, int b) { return RGB(r, g, b); }

class Pen : public GdiObj {
public:
	Pen(COLORREF color, PenStyle style = PenStyle::Solid, int width = 1) : GdiObj(error::throw_or_result(CreatePen(static_cast<int>(style), width, color))) {}
};

class DC {
public:
	DC(HDC hdc) : hdc(hdc) {}
	operator HDC() const { return hdc; }
	HGDIOBJ SelectObject(HGDIOBJ obj) const { return error::throw_or_result(::SelectObject(hdc, obj)); }
	void MoveTo(int x, int y) const { error::throw_or_result(::MoveToEx(hdc, x, y, nullptr)); }
	POINT MoveToEx(int x, int y) const {
		POINT pt;
		error::throw_or_result(::MoveToEx(hdc, x, y, &pt));
		return pt;
	}
	void LineTo(int x, int y) const { error::throw_or_result(::LineTo(hdc, x, y)); }
	COLORREF SetPenColor(COLORREF color) const { return error::throw_or_result(::SetDCPenColor(*this, color), invalid_color_error_check); }
	COLORREF SetPixel(int x, int y, COLORREF color) const { return error::throw_or_result(::SetPixel(*this, x, y, color), invalid_color_error_check); }
	void FillRect(RECT& rc, HBRUSH brush) const { error::throw_or_result(::FillRect(*this, &rc, brush)); }
private:
	HDC hdc;
};

class PaintDC : private PAINTSTRUCT, public DC {
public:
	~PaintDC() { EndPaint(hWnd, this); }
	PaintDC(const PaintDC&) = delete;
	PaintDC& operator=(const PaintDC&) = delete;
	PaintDC(PaintDC&&) = default;
	PaintDC& operator=(PaintDC&&) = default;
	const PAINTSTRUCT* operator ->() const { return this; }
	static PaintDC BeginPaint(HWND hWnd) { return PaintDC(hWnd); }
private:
	PaintDC(HWND hWnd) : DC(error::throw_or_result(::BeginPaint(hWnd, this))), hWnd(hWnd) {}
	HWND hWnd;
};

}

#endif /* WIN32_GDI_H_ */
