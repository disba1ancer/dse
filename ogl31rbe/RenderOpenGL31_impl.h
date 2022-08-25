/*
 * RenderOpenGLimpl.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_RENDEROPENGL_IMPL_H_
#define SUBSYS_RENDEROPENGL_IMPL_H_

#include <dse/renders/RenderOpenGL31.h>
#include <dse/notifier/notifier.h>
#include <dse/util/functional.h>
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
#include <dse_config.h>
#include <dse/core/ThreadPool.h>
#include "gl31/TextureInstance.h"

namespace dse::ogl31rbe {

class RenderOpenGL31_impl {
	core::Window* wnd;
	notifier::connection<core::Window::PaintHandler> paintCon;
	notifier::connection<core::Window::ResizeHandler> sizeCon;
	notifier::connection<core::Scene::ChangeEvent> scnChangeCon;
	glwrp::Context context;
	glwrp::VAO vao;
	core::Scene* scene = nullptr;
	core::Camera* camera = nullptr;
	std::unordered_map<core::ITextureDataProvider*, gl31::TextureInstance> textures;
	std::unordered_map<core::Material*, gl31::MaterialInstance> materials;
	std::unordered_map<core::IMesh*, gl31::MeshInstance> meshes;
	std::unordered_map<core::Object*, gl31::ObjectInstance> objects;
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
	std::unordered_map<core::IMesh*, gl31::MeshInstance>::iterator cleanupPointer;

	void OnPaint(core::WndEvtDt);
	void OnResize(core::WndEvtDt, int width, int height, core::WindowShowCommand);
	void RebuildSrgbFrameBuffer();
	void PrepareShaders();
	void RebuildViewport(unsigned width, unsigned height);
	void SetupCamera();
	void DrawPostprocess();
	void FillInstances();
	void OnSceneChanged(core::SceneChangeEventType act, core::Object* obj);
	void CleanupMeshes();
	void DrawScene();
public:
	RenderOpenGL31_impl(core::Window& wnd);
	~RenderOpenGL31_impl() = default;
	RenderOpenGL31_impl(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl(RenderOpenGL31_impl &&other) = delete;
	RenderOpenGL31_impl& operator=(const RenderOpenGL31_impl &other) = delete;
	RenderOpenGL31_impl& operator=(RenderOpenGL31_impl &&other) = delete;
	void Render(const util::FunctionPtr<void()>& cb);
	void SetScene(dse::core::Scene& scene);
	void SetCamera(dse::core::Camera& camera);
	auto GetObjectInstance(core::Object* object) -> gl31::ObjectInstance*;
	auto GetMeshInstance(core::IMesh* mesh, bool withAcquire = false) -> gl31::MeshInstance*;
	auto GetMaterialInstance(core::Material* material, bool withAcquire = false) -> gl31::MaterialInstance*;
	auto GetTextureInstance(core::ITextureDataProvider* texture, bool withAcquire = false) -> gl31::TextureInstance*;
private:
	void PrepareSamplers();
};

} /* namespace dse::ogl31rbe */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
