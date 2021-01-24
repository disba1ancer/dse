/*
 * Context.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include <cstring>
//#include <gl/wgl.h>
#include <exception>
#include <iostream>
#include "Context.h"

#ifdef _WIN32
#include <dwmapi.h>
#include "os/WindowData_win32.h"
#endif

namespace dse::renders::glwrp {

namespace {

#ifdef _WIN32
const int pixelFormatAtributes[] =
{
	WGL_DRAW_TO_WINDOW_ARB, TRUE,
	WGL_SUPPORT_OPENGL_ARB, TRUE,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_DOUBLE_BUFFER_ARB, TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_DEPTH_BITS_ARB, 24,
	WGL_STENCIL_BITS_ARB, 8,
	0, 0
};
#endif

struct GLVersion {
	int major;
	int minor;
};

GLVersion getVersion(ContextVersion ver) {
	switch (ver) {
#define GENERATE(major, minor) case ContextVersion::gl ## major ## minor :\
			return { major, minor }
		GENERATE(1, 0);
		GENERATE(1, 1);
		GENERATE(1, 2);
		GENERATE(1, 3);
		GENERATE(1, 4);
		GENERATE(1, 5);
		GENERATE(2, 0);
		GENERATE(2, 1);
		GENERATE(3, 0);
		GENERATE(3, 1);
		GENERATE(3, 2);
		GENERATE(3, 3);
		GENERATE(4, 0);
		GENERATE(4, 1);
		GENERATE(4, 2);
		GENERATE(4, 3);
		GENERATE(4, 4);
		GENERATE(4, 5);
		GENERATE(4, 6);
#undef GENERATE
		default:
			return { 1, 0 };
	}
}

}

#ifdef _WIN32
Context::ContextOwner::ContextOwner() noexcept : TrvMvOnlyRes(0) {}
Context::ContextOwner::ContextOwner(HGLRC glrc) noexcept : TrvMvOnlyRes(glrc) {}
Context::ContextOwner::~ContextOwner() { wglDeleteContext(resource); }

thread_local HGLRC Context::currentContext = 0;
thread_local HDC Context::currentContextDC = 0;

Context::Context(os::Window& wnd, ContextVersion ver, ContextFlags flags) : dc(swal::Wnd(wnd.getSysData().hWnd).GetDC()) {
	if (ver == ContextVersion::legacy) {
		glrc = makeLegacyContext(dc);
		MakeCurrent();
	} else {
		if (!(wglChoosePixelFormatARB && wglCreateContextAttribsARB)) {
			os::Window wnd;
			Context context(wnd, ContextVersion::legacy, ContextFlags(0));
			try {
				std::cout << wglGetExtensionsStringARB(dc) << std::endl;
			} catch(...) {}
			wglChoosePixelFormatARB.load();
			wglCreateContextAttribsARB.load();
		}
		glrc = makeContext(dc, ver, flags);
		MakeCurrent();
	}
	[[maybe_unused]] static bool init = [](){
		glbinding::initialize(getProcAddress, false);
		return true;
	}();
}

Context::~Context() {
	if (currentContext == glrc) {
		MakeCurrentEmpty();
	}
}

void Context::SwapBuffers() {
	::SwapBuffers(dc);
	if (vsync) DwmFlush();
}

void Context::enableVSync(int val) {
	vsync = val;
	wglSwapIntervalEXT(0);
}

auto Context::getProcAddress(const char *name) -> void(*)() {
	return reinterpret_cast<void(*)()>(wglGetProcAddress(name));
}

Context::ContextOwner Context::makeLegacyContext(swal::WindowDC& dc) {
	PIXELFORMATDESCRIPTOR pfd{};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int format = swal::winapi_call(ChoosePixelFormat(dc, &pfd));
	swal::winapi_call(DescribePixelFormat(dc, format, sizeof(pfd), &pfd));
	if (pfd.dwFlags & (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)) throw std::runtime_error("Pixel format not accelerated (legacy context)");
	swal::winapi_call(SetPixelFormat(dc, format, &pfd));
	return swal::winapi_call(wglCreateContext(dc));
}

Context::ContextOwner Context::makeContext(swal::WindowDC &dc, ContextVersion ver, ContextFlags flags) {
	PIXELFORMATDESCRIPTOR pfd{};
	UINT formatCount;
	int format;
	swal::winapi_call(wglChoosePixelFormatARB(dc, pixelFormatAtributes, nullptr, 1, &format, &formatCount));
	swal::winapi_call(DescribePixelFormat(dc, format, sizeof(pfd), &pfd));
	if (pfd.dwFlags & (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)) throw std::runtime_error("Pixel format not accelerated");
	swal::winapi_call(SetPixelFormat(dc, format, nullptr));
	auto [major, minor] = getVersion(ver);
	int contextAtributes[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_FLAGS_ARB, int(flags),//WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0, 0
	};
	return swal::winapi_call(wglCreateContextAttribsARB(dc, 0, contextAtributes));
}

void Context::MakeCurrent(os::Window &wnd) {
	dc = swal::Wnd(wnd.getSysData().hWnd).GetDC();
	MakeCurrent();
}

void Context::MakeCurrent() {
	if (currentContext != glrc || currentContextDC != dc) {
		swal::winapi_call(wglMakeCurrent(dc, glrc));
		currentContext = glrc;
		currentContextDC = dc;
		glbinding::useContext(glbinding::ContextHandle(HGLRC(glrc)));
	}
}

void Context::MakeCurrentEmpty() {
	currentContext = 0;
	currentContextDC = 0;
	wglMakeCurrent(0, 0);
	glbinding::useContext(0);
}
#endif

} /* namespace dse::renders::glwrp */
