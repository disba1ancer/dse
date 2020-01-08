/*
 * RenderOpenGLimpl.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_RENDEROPENGL_IMPL_H_
#define SUBSYS_RENDEROPENGL_IMPL_H_

#include "RenderOpenGL31.h"
#include "../notifier/notifier.h"
#include "../notifier/make_handler.h"
#include "gl/Context31.h"

namespace dse {
namespace subsys {

class RenderOpenGL31_impl {
	os::Window* wnd;
	notifier::connection<os::Window::PaintHandler> paintCon;
	notifier::connection<os::Window::ResizeHandler> sizeCon;
	gl::Context31 context;

	void onPaint(os::WndEvtDt, os::PntEvtDt);
	void onResize(os::WndEvtDt, int width, int height, os::WindowShowCommand);
public:
	RenderOpenGL31_impl(os::Window& wnd);
	~RenderOpenGL31_impl() = default;
	RenderOpenGL31_impl(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl(RenderOpenGL31_impl &&other) = delete;
	RenderOpenGL31_impl& operator=(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl& operator=(RenderOpenGL31_impl &&other) = delete;
	bool renderTask();
};

} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
