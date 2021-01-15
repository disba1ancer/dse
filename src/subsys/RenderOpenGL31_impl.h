/*
 * RenderOpenGLimpl.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_RENDEROPENGL_IMPL_H_
#define SUBSYS_RENDEROPENGL_IMPL_H_

#include "RenderOpenGL31.h"
#include "notifier/notifier.h"
#include "util/functional.h"
#include "gl/Context31.h"
#include "gl/VAO.h"
#include "util/ExecutionThread.h"
#include <vector>
#include <list>
#include "gl31_impl/MeshInstance.h"
#include "gl31_impl/ObjectInstance.h"
#include "gl/Program.h"
#include "gl/Buffer.h"
#include "gl/Shader.h"
#include "gl/FrameBuffer.h"
#include "gl/Texture.h"
#include "gl/RenderBuffer.h"
#include "dse_config.h"
#include "core/ThreadPool.h"

namespace dse {
namespace subsys {

class RenderOpenGL31_impl {
	os::Window* wnd;
	notifier::connection<os::Window::PaintHandler> paintCon;
	notifier::connection<os::Window::ResizeHandler> sizeCon;
	gl::Context31 context;
	gl::VAO vao;
	scn::Scene* scene = nullptr;
	scn::Camera* camera = nullptr;
	std::vector<gl31_impl::ObjectInstance> objects;
	std::list<gl31_impl::MeshInstance> meshes;
	gl::VertexBuffer vbo;
	gl::Program fragmentProg;
	gl::Program drawProg;
	GLint fragWindowSizeUniform = 0;
	GLint fragColorBufferUniform = 0;
	GLint fragDepthBufferUniform = 0;
	GLint drawWindowSizeUniform = 0;
	GLint posUniform = 0;
	GLint qRotUniform = 0;
	GLint scaleUniform = 0;
	GLint camPosUniform = 0;
	GLint camQRotUniform = 0;
	GLint matColorUnifrom = 0;
	GLint invAspRatioUniform = 0;
	GLint perspArgsUniform = 0;
	unsigned width = 1, height = 1;
#ifdef DSE_MULTISAMPLE
	gl::FrameBuffer renderFBOMSAA = 0;
	gl::RenderBuffer colorBufferMSAA = 0;
	gl::RenderBuffer depthBufferMSAA = 0;
#endif
	gl::FrameBuffer renderFBO = 0;
	gl::TextureRectangle colorBuffer = 0;
	gl::TextureRectangle depthBuffer = 0;
	util::promise<void> pr;
	std::atomic_bool requested = false;

	void onPaint(os::WndEvtDt);
	void onResize(os::WndEvtDt, int width, int height, os::WindowShowCommand);
	void rebuildSrgbFrameBuffer();
	void prepareShaders();
	void resumeRenderCaller();
public:
	RenderOpenGL31_impl(os::Window& wnd);
	~RenderOpenGL31_impl() = default;
	RenderOpenGL31_impl(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl(RenderOpenGL31_impl &&other) = delete;
	RenderOpenGL31_impl& operator=(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl& operator=(RenderOpenGL31_impl &&other) = delete;
	auto render() -> util::future<void>;
	void setScene(dse::scn::Scene& scene);
	void setCamera(dse::scn::Camera& camera);
};

} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
