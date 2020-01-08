/*
 * RenderOpenGLimpl.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "../os/WindowData_win32.h"
#include "RenderOpenGL31_impl.h"
#include "gl/gl.h"

namespace dse {
namespace subsys {

void RenderOpenGL31_impl::onPaint(os::WndEvtDt, os::PntEvtDt) {
	glClear(GL_COLOR_BUFFER_BIT);
	context.SwapBuffers();
}

RenderOpenGL31_impl::RenderOpenGL31_impl(os::Window& wnd) : wnd(&wnd),
		paintCon(wnd.subscribePaintEvent(notifier::make_handler<&onPaint>(this))),
		sizeCon(wnd.subscribeResizeEvent(notifier::make_handler<&onResize>(this))),
		context(wnd) {
	glClearColor(0.f, 0.f, .4f, 1.f);
}

void RenderOpenGL31_impl::onResize(os::WndEvtDt, int width, int height,
		os::WindowShowCommand) {
	glViewport(0, 0, width, height);
}

bool RenderOpenGL31_impl::renderTask() {
#ifdef _WIN32
	auto hWnd = wnd->getSysData().hWnd;
	InvalidateRect(hWnd, nullptr, FALSE);
	UpdateWindow(hWnd);
#endif
	return true;
}

} /* namespace subsys */
} /* namespace dse */
