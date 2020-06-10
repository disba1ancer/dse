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
#include "gl/VAO.h"
#include "../threadutils/ExecutionThread.h"
#include <vector>
#include <list>
#include "gl31_impl/MeshInstance.h"
#include "gl31_impl/ObjectInstance.h"
#include "gl/Program.h"
#include "gl/Buffer.h"
#include "gl/VertexShader.h"
#include "gl/FragmentShader.h"

namespace dse {
namespace subsys {

class RenderOpenGL31_impl {
	os::Window* wnd;
	notifier::connection<os::Window::PaintHandler> paintCon;
	notifier::connection<os::Window::ResizeHandler> sizeCon;
	gl::Context31 context;
	gl::VAO vao;
	scn::Scene* scene;
	scn::Camera* camera;
	std::vector<gl31_impl::ObjectInstance> objects;
	std::list<gl31_impl::MeshInstance> meshes;
	gl::VertexBuffer vbo;
	gl::Program fragmentProg;
	gl::Program drawProg;
	GLint fragWindowSizeUniform;
	GLint drawWindowSizeUniform;
	GLint posUniform;
	GLint qRotUniform;
	GLint scaleUniform;
	GLint camPosUniform;
	GLint camQRotUniform;
	GLint invFocLenUniform;
	GLint zNearUniform;
	GLint zFarUniform;
	GLint aspRatioUniform;

	void onPaint(os::WndEvtDt, os::PntEvtDt);
	void onResize(os::WndEvtDt, int width, int height, os::WindowShowCommand);
public:
	RenderOpenGL31_impl(os::Window& wnd);
	~RenderOpenGL31_impl() = default;
	RenderOpenGL31_impl(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl(RenderOpenGL31_impl &&other) = delete;
	RenderOpenGL31_impl& operator=(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl& operator=(RenderOpenGL31_impl &&other) = delete;
	threadutils::TaskState renderTask();
	void setScene(dse::scn::Scene& scene);
	void setCamera(dse::scn::Camera& camera);
};

} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
