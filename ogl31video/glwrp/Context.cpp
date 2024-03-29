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
#include <dse/core/WindowData_win32.h>
#include <dwmapi.h>
#endif

namespace dse::ogl31rbe::glwrp {

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
	WGL_ALPHA_BITS_ARB, 8,
	WGL_DEPTH_BITS_ARB, 24,
	WGL_STENCIL_BITS_ARB, 8,
	WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, FALSE,
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
Context::ContextOwner::~ContextOwner() {
	if (resource) {
		swal::winapi_call(wglDeleteContext(resource));
	}
}

thread_local Context* Context::currentContext = nullptr;

Context::Context(core::Window& wnd, ContextVersion ver, ContextFlags flags) : dc(swal::Wnd(wnd.GetSysData().hWnd).GetDC()) {
    if (ver == ContextVersion::legacy) {
		glrc = makeLegacyContext(dc);
	} else {
		if (!(wglChoosePixelFormatARB && wglCreateContextAttribsARB)) {
			core::Window wnd;
			Context context(wnd, ContextVersion::legacy, ContextFlags(0));
			try {
				std::cout << wglGetExtensionsStringARB(dc) << std::endl;
			} catch(...) {}
			wglChoosePixelFormatARB.load();
			wglCreateContextAttribsARB.load();
		}
		glrc = makeContext(dc, ver, flags);
	}
	glbinding::initialize(glbinding::ContextHandle(HGLRC(glrc)), &Context::getProcAddress, false, false);
	MakeCurrent();
}

Context::~Context() {
	glbinding::releaseContext(glbinding::ContextHandle(HGLRC(glrc)));
	if (currentContext == this) {
		MakeCurrentEmpty();
	}
}

void Context::SwapBuffers() {
	::SwapBuffers(dc);
	if (vsync) DwmFlush();
}

void Context::enableVSync(int val) {
	vsync = val;
//	wglSwapIntervalEXT(val);
}

auto Context::getProcAddress(const char *name) -> void(*)() {
	auto f = reinterpret_cast<void(*)()>(wglGetProcAddress(name));
	if (!f) {
		static auto glh = swal::winapi_call(GetModuleHandle(TEXT("opengl32.dll")));
		f = reinterpret_cast<void(*)()>(GetProcAddress(glh, name));
	}
	return f;
}

Context::ContextOwner Context::makeLegacyContext(swal::WindowDC& dc) {
	PIXELFORMATDESCRIPTOR pfd{};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;
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
	int profileMask = 0;
	int profileMaskName = 0;
	if (major >= 3 && minor >= 2) {
		profileMask = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		profileMaskName = WGL_CONTEXT_PROFILE_MASK_ARB;
	}
	int contextAtributes[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_FLAGS_ARB, int(flags),//WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		profileMaskName, profileMask,
		0, 0
	};
	return swal::winapi_call(wglCreateContextAttribsARB(dc, 0, contextAtributes));
}

void Context::MakeCurrent(core::Window &wnd) {
	auto newDC = swal::Wnd(wnd.GetSysData().hWnd).GetDC();
	if (currentContext != this || dc.get() != newDC.get()) {
		dc = std::move(newDC);
		MakeCurrentInternal();
	}
}

void Context::MakeCurrent() {
	if (currentContext != this) {
		MakeCurrentInternal();
	}
}

void Context::MakeCurrentEmpty() {
	currentContext = nullptr;
	wglMakeCurrent(0, 0);
}

void Context::MakeCurrentInternal() {
	swal::winapi_call(wglMakeCurrent(dc, glrc));
	currentContext = this;
	glbinding::useContext(glbinding::ContextHandle(HGLRC(glrc)));
}
#endif

} /* namespace dse::ogl31rbe::glwrp */
