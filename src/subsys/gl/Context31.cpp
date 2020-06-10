/*
 * Context.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include <cstring>
#include "../../os/WindowData_win32.h"
#include "gl.h"
#include <exception>
#include "Context31.h"
#include "dwmapi.h"

namespace {

const int pixelFormatAtributes[] =
{
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_DEPTH_BITS_ARB, 24,
	WGL_STENCIL_BITS_ARB, 8,
	0, 0
};

const int contextAtributes[] =
{
	WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
	WGL_CONTEXT_MINOR_VERSION_ARB, 1,
	//WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
	0, 0
};

}

namespace dse {
namespace subsys {
namespace gl {

#ifdef _WIN32
Context31::Context31(os::Window& wnd) : hWnd(wnd.getSysData().hWnd), hdc(GetDC(hWnd)) {
	PIXELFORMATDESCRIPTOR pfd{};
	if (!(wglChoosePixelFormatARB && wglCreateContextAttribsARB)) {
		os::Window wnd;
		HDC hdc = GetDC(wnd.getSysData().hWnd);
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int format = ChoosePixelFormat(hdc, &pfd);
		DescribePixelFormat(hdc, format, sizeof(pfd), &pfd);
		if (pfd.dwFlags & (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)) throw std::runtime_error("Pixel format not accelerated");
		SetPixelFormat(hdc, format, &pfd);
		HGLRC glrc = wglCreateContext(hdc);
		wglMakeCurrent(hdc, glrc);
		wglChoosePixelFormatARB.load();
		wglCreateContextAttribsARB.load();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(glrc);
	}
	UINT formatCount;
	int format;
	wglChoosePixelFormatARB(hdc, pixelFormatAtributes, nullptr, 1, &format, &formatCount);
	DescribePixelFormat(hdc, format, sizeof(pfd), &pfd);
	if (pfd.dwFlags & (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)) throw std::runtime_error("Pixel format not accelerated");
	SetPixelFormat(hdc, format, nullptr);
	glrc = wglCreateContextAttribsARB(hdc, 0, contextAtributes);
	if (!glrc) throw std::runtime_error("Unable to initialize OpenGL");
	wglMakeCurrent(hdc, glrc);
}

Context31::~Context31() {
	wglMakeCurrent(0, 0);
	wglDeleteContext(glrc);
	ReleaseDC(hWnd, hdc);
}

void Context31::SwapBuffers() {
	if (vsync) DwmFlush();
	::SwapBuffers(hdc);
}

void Context31::enableVSync(int val) {
	vsync = val;
	wglSwapIntervalEXT(val);
}
#endif

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */
