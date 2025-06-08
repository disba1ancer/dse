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
#include "MeshInstance.h"
#include "ObjectInstance.h"
#include "MaterialInstance.h"
#include "glwrp/Program.h"
#include "glwrp/Buffer.h"
#include "glwrp/Sampler.h"
#include "glwrp/Shader.h"
#include "glwrp/FrameBuffer.h"
#include "glwrp/Texture.h"
#include "glwrp/RenderBuffer.h"
#include <dse_config.h>
#include <dse/core/ThreadPool.h>
#include "TextureInstance.h"
#include "../../core/win32.h"
#include <dse/core/Window_win32.h>

namespace dse::ogl31rbe {

class RenderOpenGL31_impl {
    void OnPaint(HWND hWnd, WPARAM, LPARAM);
    void OnResize();
    void RebuildSrgbFrameBuffer();
    void PrepareShaders();
    void RebuildViewport(unsigned width, unsigned height);
    void SetupCamera();
    void DrawPostprocess();
    void FillInstances();
    void OnSceneObjectAdd(core::IScene& scene, core::ISceneObject &obj);
    void OnSceneObjectMod(core::IScene& scene, core::ISceneObject &obj);
    void OnSceneObjectDel(core::IScene& scene, core::ISceneObject &obj);
    void OnSceneObjectsClr(core::IScene& scene);
    void CleanupMeshes();
    void DrawScene();
    auto DrawTypeToGL(core::IMesh2::Draw drawType) -> gl::GLenum;
    void PrepareSamplers();
public:
    RenderOpenGL31_impl(core::Window& wnd);
    ~RenderOpenGL31_impl() = default;
    RenderOpenGL31_impl(const RenderOpenGL31_impl &other) = delete;
    RenderOpenGL31_impl(RenderOpenGL31_impl &&other) = delete;
    RenderOpenGL31_impl& operator=(const RenderOpenGL31_impl &other) = delete;
    RenderOpenGL31_impl& operator=(RenderOpenGL31_impl &&other) = delete;
    void Render();
    void SetScene(dse::core::IScene* scene);
    void SetCamera(dse::core::Camera& camera);
    auto GetObjectInstance(core::ISceneObject* object) -> gl31::ObjectInstance*;
    auto GetMeshInstance(core::IMesh2* mesh, bool withAcquire = false) -> gl31::MeshInstance*;
    auto GetMaterialInstance(core::IMaterial* material, bool withAcquire = false) -> gl31::MaterialInstance*;
    auto GetTextureInstance(core::ITexture *texture, bool withAcquire = false) -> gl31::TextureInstance*;
private:
    core::Window* wnd;
    using howner = core::Window::handle_owner;
    howner hPaint = wnd->SubscribeEvent<core::WindowEvent::System + WM_PAINT>({*this, util::fn_tag<&RenderOpenGL31_impl::OnPaint>});
    howner hResize = wnd->SubscribeEvent<core::WindowEvent::Resize>({*this, util::fn_tag<&RenderOpenGL31_impl::OnResize>});
    core::IScene::handle_owner hSceneObjectAdd;
    core::IScene::handle_owner hSceneObjectMod;
    core::IScene::handle_owner hSceneObjectDel;
    core::IScene::handle_owner hSceneObjectsClr;
    glwrp::Context context;
    glwrp::VAO vao;
    core::IScene* scene = nullptr;
    core::Camera* camera = nullptr;
    std::unordered_map<core::ITexture*, gl31::TextureInstance> textures;
    std::unordered_map<core::IMaterial*, gl31::MaterialInstance> materials;
    std::unordered_map<core::IMesh2*, gl31::MeshInstance> meshes;
    std::unordered_map<const core::ISceneObject*, gl31::ObjectInstance> objects;
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
    glwrp::Texture2D defaultNormalMap = 0;
    glwrp::UniformBuffer cameraUBO = 0;
    glwrp::UniformBuffer emptyMaterialUBO = 0;
    glwrp::Sampler postProcColor = 0;
    glwrp::Sampler postProcDepth = 0;
    glwrp::Sampler drawDiffuse = 0;
    glwrp::Sampler drawNormal = 0;
    std::unordered_map<core::IMesh2*, gl31::MeshInstance>::iterator cleanupPointer;
};

} /* namespace dse::ogl31rbe */

#endif /* SUBSYS_RENDEROPENGL_IMPL_H_ */
