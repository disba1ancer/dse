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
#include "glwrp/Context.h"
#include "glwrp/VAO.h"
#include "util/ExecutionThread.h"
#include <map>
#include "gl31/MeshInstance.h"
#include "gl31/ObjectInstance.h"
#include "glwrp/Program.h"
#include "glwrp/Buffer.h"
#include "glwrp/Shader.h"
#include "glwrp/FrameBuffer.h"
#include "glwrp/Texture.h"
#include "glwrp/RenderBuffer.h"
#include "dse_config.h"
#include "core/ThreadPool.h"

namespace dse::renders {

class RenderOpenGL31_impl {
	os::Window* wnd;
	notifier::connection<os::Window::PaintHandler> paintCon;
	notifier::connection<os::Window::ResizeHandler> sizeCon;
	notifier::connection<scn::Scene::ChangeEvent> scnChangeCon;
	glwrp::Context context;
	glwrp::VAO vao;
	scn::Scene* scene = nullptr;
	scn::Camera* camera = nullptr;
	std::map<scn::IMesh*, gl31::MeshInstance> meshes;
	std::map<scn::Object*, gl31::ObjectInstance> objects;
	glwrp::VertexBuffer vbo;
	glwrp::Program fragmentProg;
	glwrp::Program drawProg;
	gl::GLint fragWindowSizeUniform = 0;
	gl::GLint fragColorBufferUniform = 0;
	gl::GLint fragDepthBufferUniform = 0;
	gl::GLint drawWindowSizeUniform = 0;
	gl::GLint matColorUnifrom = 0;
	unsigned width = 1, height = 1;
#ifdef DSE_MULTISAMPLE
	glwrp::FrameBuffer renderFBOMSAA = 0;
	glwrp::RenderBuffer colorBufferMSAA = 0;
	glwrp::RenderBuffer depthBufferMSAA = 0;
#endif
	glwrp::FrameBuffer renderFBO = 0;
	glwrp::TextureRectangle colorBuffer = 0;
	glwrp::TextureRectangle depthBuffer = 0;
	glwrp::UniformBuffer cameraUBO = 0;
	util::promise<void> pr;
	std::atomic_bool requested = false;
	std::map<scn::IMesh*, gl31::MeshInstance>::iterator cleanupPointer;

	void onPaint(os::WndEvtDt);
	void onResize(os::WndEvtDt, int width, int height, os::WindowShowCommand);
	void rebuildSrgbFrameBuffer();
	void prepareShaders();
	void resumeRenderCaller();
	void rebuildViewport(int width, int height);
	void setupCamera();
	void drawPostprocess();
	void fillInstances();
	auto getMeshInstance(scn::IMesh* mesh) -> gl31::MeshInstance*;
	void reloadInstance(gl31::ObjectInstance& objInst);
	void onSceneChanged(scn::SceneChangeEventType act, scn::Object* obj);
	void cleanupMeshes();
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

} /* namespace dse::renders */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
