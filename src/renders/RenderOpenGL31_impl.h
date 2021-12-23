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
#include <map>
#include <unordered_map>
#include "gl31/MeshInstance.h"
#include "gl31/ObjectInstance.h"
#include "gl31/MaterialInstance.h"
#include "glwrp/Program.h"
#include "glwrp/Buffer.h"
#include "glwrp/Sampler.h"
#include "glwrp/Shader.h"
#include "glwrp/FrameBuffer.h"
#include "glwrp/Texture.h"
#include "glwrp/RenderBuffer.h"
#include "dse_config.h"
#include "core/ThreadPool.h"
#include "gl31/TextureInstance.h"

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
	std::unordered_map<scn::ITextureDataProvider*, gl31::TextureInstance> textures;
	std::unordered_map<scn::Material*, gl31::MaterialInstance> materials;
	std::unordered_map<scn::IMesh*, gl31::MeshInstance> meshes;
	std::unordered_map<scn::Object*, gl31::ObjectInstance> objects;
	glwrp::VertexBuffer vbo;
	glwrp::Program fragmentProg;
	glwrp::Program drawProg;
	gl::GLint fragWindowSizeUniform = 0;
	gl::GLint drawWindowSizeUniform = 0;
	unsigned width = 1, height = 1;
#ifdef DSE_MULTISAMPLE
	glwrp::FrameBuffer renderFBOMSAA = 0;
	glwrp::RenderBuffer colorBufferMSAA = 0;
	glwrp::RenderBuffer depthBufferMSAA = 0;
#endif
	glwrp::FrameBuffer renderFBO = 0;
	glwrp::Texture2D colorBuffer = 0;
	glwrp::Texture2D depthBuffer = 0;
	glwrp::Texture2D pendingTexture = 0;
	glwrp::UniformBuffer cameraUBO = 0;
	glwrp::UniformBuffer emptyMaterialUBO = 0;
	glwrp::Sampler postProcColor = 0;
	glwrp::Sampler postProcDepth = 0;
	glwrp::Sampler drawDiffuse = 0;
	util::FunctionPtr<void()> renderCallback;
	std::atomic_bool requested = false;
	std::unordered_map<scn::IMesh*, gl31::MeshInstance>::iterator cleanupPointer;

	void OnPaint(os::WndEvtDt);
	void OnResize(os::WndEvtDt, int width, int height, os::WindowShowCommand);
	void RebuildSrgbFrameBuffer();
	void PrepareShaders();
	void RebuildViewport(unsigned width, unsigned height);
	void SetupCamera();
	void DrawPostprocess();
	void FillInstances();
	void OnSceneChanged(scn::SceneChangeEventType act, scn::Object* obj);
	void CleanupMeshes();
	void DrawScene();
public:
	RenderOpenGL31_impl(os::Window& wnd);
	~RenderOpenGL31_impl() = default;
	RenderOpenGL31_impl(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl(RenderOpenGL31_impl &&other) = delete;
	RenderOpenGL31_impl& operator=(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl& operator=(RenderOpenGL31_impl &&other) = delete;
	void Render(const util::FunctionPtr<void()>& cb);
	void SetScene(dse::scn::Scene& scene);
	void SetCamera(dse::scn::Camera& camera);
	auto GetObjectInstance(scn::Object* object) -> gl31::ObjectInstance*;
	auto GetMeshInstance(scn::IMesh* mesh, bool withAcquire = false) -> gl31::MeshInstance*;
	auto GetMaterialInstance(scn::Material* material, bool withAcquire = false) -> gl31::MaterialInstance*;
	auto GetTextureInstance(scn::ITextureDataProvider* texture, bool withAcquire = false) -> gl31::TextureInstance*;
private:
	void PrepareSamplers();
};

} /* namespace dse::renders */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
