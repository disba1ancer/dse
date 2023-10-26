/*
 * RenderOpenGLimpl.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include <array>
#include <cstdio>
#include <cstring>
#include <glbinding/gl31/gl.h>
#include <glbinding/gl31ext/gl.h>
#include "RenderOpenGL31_impl.h"
#include <dse/core/win32.h>
#include <dse/core/WindowData_win32.h>
#include "glwrp/gl.h"
#include <dse_shaders/gl31.h>
#include "gl31/binds.h"
#include <dse/core/Material.h>
#include <dse/math/qmath.h>
#include <iostream>

namespace {
struct ReqGLExt {
    const char* name;
    bool avail;
};
auto reqGLExts = std::to_array<ReqGLExt>({
    {"GL_ARB_draw_elements_base_vertex", false},
    {"GL_ARB_instanced_arrays", false},
    {"GL_ARB_sampler_objects", false},
});
using dse::ogl31rbe::gl31::InputParams;
using dse::ogl31rbe::gl31::OutputParams;
using dse::ogl31rbe::gl31::UniformIndices;
using dse::ogl31rbe::gl31::ObjectInstanceUniform;
using dse::ogl31rbe::gl31::CameraUniform;
using dse::ogl31rbe::gl31::TextureUnits;
using namespace gl31;
using namespace gl31ext;
dse::math::vec3 fullscreenPrimitive[] = { {-1.f, -1.f, -1.f}, {3.f, -1.f, -1.f}, {-1.f, 3.f, -1.f} };
auto pendingTextureData = std::to_array<unsigned char>({
    40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240,
    40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240,
    240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40,
    240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40,
    40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240,
    40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240,
    240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40,
    240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40, 240, 40, 240, 240, 40, 240, 40, 40, 40, 40, 40, 40,
});
auto defaultNormalMapData = std::to_array<unsigned char>({ 255, 128, 128, 255 });
}

//extern "C" __declspec(dllexport) int NvOptimusEnablement = 1;

namespace dse::ogl31rbe {

void RenderOpenGL31_impl::SetupCamera() {
    CameraUniform uniforms = {};
    auto& camPos = uniforms.pos;
    camPos["xyz"] = camera->getPos();
    camPos.w() = 1.f;
    auto& viewProj = uniforms.viewProj;
    math::mat3 mRot = matFromQuat(qinv(camera->getRot()));
    viewProj[0]["xyz"] = mRot[0];
    viewProj[1]["xyz"] = mRot[1];
    viewProj[2]["xyz"] = mRot[2];
    viewProj[3]["xyz"] = -(mRot * uniforms.pos["xyz"]);
    viewProj[3].w() = 1.f;
    auto zNear = camera->getNear(),
        zFar = camera->getFar();
    if (zNear == zFar) {
        zNear = 1.f;
        zFar = 131072.f;
    }
    float c = -1.f / camera->getFocalLength();
    float a = c / (zFar - zNear);
    float b = 2 * zFar * zNear * a;
    a *= (zFar + zNear);
    math::vec4 persp = {float(height) / float(width), 1.f, a, c};
    viewProj[0] = viewProj[0]["xyzz"] * persp;
    viewProj[1] = viewProj[1]["xyzz"] * persp;
    viewProj[2] = viewProj[2]["xyzz"] * persp;
    viewProj[3] = viewProj[3]["xyzz"] * persp;
    viewProj[3].z() += b;
    cameraUBO.bind();
    glBufferSubData(cameraUBO.target, 0, sizeof(CameraUniform), &uniforms);
    glBindBufferBase(cameraUBO.target, UniformIndices::CameraBind, cameraUBO);
}

void RenderOpenGL31_impl::DrawPostprocess() {
#ifdef DSE_MULTISAMPLE
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
    glBindSampler(TextureUnits::PostProcColor, postProcColor);
    glBindSampler(TextureUnits::PostProcDepth, postProcDepth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthFunc(GL_ALWAYS);
    glActiveTexture(texture(TextureUnits::PostProcColor));
    colorBuffer.bind();
    glActiveTexture(texture(TextureUnits::NullImageUnit));
    depthBuffer.bind();
    glUseProgram(fragmentProg);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthFunc(GL_LESS);
}

void RenderOpenGL31_impl::OnSceneChanged(core::SceneChangeEventType act, core::Object* obj) {
    switch (act) {
        case decltype(act)::ObjectCreate: {
            auto& inst = objects[obj];
            inst = gl31::ObjectInstance(obj);
        } break;
        case decltype(act)::ObjectDestroy: {
            objects.erase(obj);
        } break;
    }
}

void RenderOpenGL31_impl::CleanupMeshes()
{
    auto end = meshes.end();
    if (cleanupPointer == end) {
        cleanupPointer = meshes.begin();
    }
    for (int i = 0; i < DSE_RENDER_GC_OBJECTS_PER_FRAME && cleanupPointer != end; ++i) {
        if (cleanupPointer->second.isNoRefs()) {
            meshes.erase(cleanupPointer++);
        } else {
            ++cleanupPointer;
        }
    }
}

void RenderOpenGL31_impl::DrawScene()
{
    glUseProgram(drawProg);
    glBindSampler(TextureUnits::DrawDiffuse, drawDiffuse);
    glBindSampler(TextureUnits::DrawNormalMap, drawNormal);
    glActiveTexture(texture(TextureUnits::DrawDiffuse));
    pendingTexture.bind();
    glActiveTexture(texture(TextureUnits::DrawNormalMap));
    defaultNormalMap.bind();
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (!camera) {
        return;
    }
    SetupCamera();
    if (!scene) {
        return;
    }
    for (auto& obj : scene->objects()) {
        auto inst = GetObjectInstance(&obj);
        inst->CheckAndSync(this);
        auto mesh = inst->GetMeshInstance();
        if (!mesh) {
            continue;
        }
        if (!mesh->IsReady()) {
            continue;
        }
        glBindVertexArray(mesh->GetVAO());
        auto& ubo = inst->GetUBO();
        glBindBufferBase(ubo.target, UniformIndices::ObjectInstanceBind, ubo);
        auto subCount = mesh->GetSubmeshCount();
        for (std::size_t i = 0; i < subCount; ++i) {
            auto [start, end, drawType] = mesh->GetSubmeshRange(i);
            auto materialInst = inst->GetMaterialInstance(this, i);
            if (materialInst) {
                materialInst->CheckAndSync(this);
                auto& matUbo = materialInst->GetUBO();
                glBindBufferBase(matUbo.target, UniformIndices::MaterialBind, matUbo);
                auto textureInst = materialInst->GetDiffuseTextureInstance(this);
                glActiveTexture(texture(gl31::DrawDiffuse));
                if (textureInst == nullptr || !textureInst->IsReady()) {
                    pendingTexture.bind();
                } else {
                    textureInst->GetTexture().bind();
                }
                textureInst = materialInst->GetNormalmapInstance(this);
                glActiveTexture(texture(gl31::DrawNormalMap));
                if (textureInst == nullptr || !textureInst->IsReady()) {
                    defaultNormalMap.bind();
                } else {
                    textureInst->GetTexture().bind();
                }
            } else {
                glBindBufferBase(emptyMaterialUBO.target, UniformIndices::MaterialBind, emptyMaterialUBO);
                glActiveTexture(texture(gl31::DrawDiffuse));
                pendingTexture.bind();
            }
            glDrawElements(DrawTypeToGL(drawType), end - start, GL_UNSIGNED_INT, reinterpret_cast<void*>(start * sizeof(std::uint32_t)));
        }
    }
}

auto RenderOpenGL31_impl::DrawTypeToGL(core::IMesh::Draw drawType) -> gl::GLenum
{
    using DT = core::IMesh::Draw;
    switch (drawType) {
    case DT::Triangles:
        return GL_TRIANGLES;
    case DT::Stripes:
        return GL_TRIANGLE_STRIP;
    }
    throw std::runtime_error("Improper mesh draw type");
}

void RenderOpenGL31_impl::FillInstances()
{
    for (auto& object : (scene->objects())) {
        auto& objInst = objects[&object];
        objInst = gl31::ObjectInstance(this, &object);
    }
//    scnChangeCon = scene->subscribeChangeEvent(util::StaticMemFn<&RenderOpenGL31_impl::OnSceneChanged>(*this));
}

auto RenderOpenGL31_impl::GetMeshInstance(core::IMesh* mesh, bool withAcquire) -> gl31::MeshInstance*
{
    gl31::MeshInstance* result;
    if (!mesh) return nullptr;
    auto it = meshes.find(mesh);
    if (it == meshes.end()) {
        auto [it2, emplaceResult] = meshes.emplace(
            std::piecewise_construct,
            std::make_tuple(mesh),
            std::make_tuple(mesh)
        );
        if (!emplaceResult) {
            throw std::bad_alloc();
        }
        it = it2;
    }
    result = &(it->second);
    if (withAcquire) {
        result->AddRef();
    }
    return result;
}

auto RenderOpenGL31_impl::GetMaterialInstance(core::Material* material, bool withAcquire) -> gl31::MaterialInstance*
{
    gl31::MaterialInstance* result;
    if (!material) return nullptr;
    auto it = materials.find(material);
    if (it == materials.end()) {
        auto [it2, emplaceResult] = materials.emplace(
            std::piecewise_construct,
            std::make_tuple(material),
            std::make_tuple(material)
        );
        if (!emplaceResult) {
            throw std::bad_alloc();
        }
        it = it2;
    }
    result = &(it->second);
    if (withAcquire) {
        result->AddRef();
    }
    return result;
}

auto RenderOpenGL31_impl::GetTextureInstance(
    core::ITextureDataProvider* texture, bool withAcquire
) -> gl31::TextureInstance* {
    gl31::TextureInstance* result;
    if (!texture) return nullptr;
    auto it = textures.find(texture);
    if (it == textures.end()) {
        auto [it2, emplaceResult] = textures.emplace(
            std::piecewise_construct,
            std::make_tuple(texture),
            std::make_tuple(texture)
        );
        if (!emplaceResult) {
            throw std::bad_alloc();
        }
        it = it2;
    }
    result = &(it->second);
    if (withAcquire) {
        result->AddRef();
    }
    return result;
}

void RenderOpenGL31_impl::OnPaint(core::WndEvtDt) {
#ifdef DSE_MULTISAMPLE
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBOMSAA);
#else
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawScene();
    DrawPostprocess();

    context.SwapBuffers();

    CleanupMeshes();

    swal::Wnd(wnd->GetSysData().hWnd).ValidateRect();
    if (requested.load(std::memory_order_acquire)) {
        auto pool = core::ThreadPool::GetCurrentPool();
        pool.Schedule(renderCallback);
        requested.store(false, std::memory_order_release);
    }
}

void RenderOpenGL31_impl::PrepareSamplers() {
    postProcColor = {};
    postProcDepth = {};
    drawDiffuse = {};
    glSamplerParameteri(drawDiffuse, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(drawDiffuse, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    drawNormal = {};
    glSamplerParameteri(drawNormal, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(drawNormal, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

RenderOpenGL31_impl::RenderOpenGL31_impl(core::Window& wnd) : wnd(&wnd),
        paintCon(wnd.SubscribePaintEvent(util::FunctionPtr{*this, util::fnTag<&RenderOpenGL31_impl::OnPaint>})),
        sizeCon(wnd.SubscribeResizeEvent(util::FunctionPtr{*this, util::fnTag<&RenderOpenGL31_impl::OnResize>})),
        context(wnd, glwrp::ContextVersion::gl31, glwrp::ContextFlags::Debug),
        cleanupPointer(meshes.end())
{
    std::cout << glGetString(GL_VERSION) << "\n";
    GLint numExts;
    unsigned availExtsNum = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
    for (int i = 0; i < numExts; ++i) {
        auto extName = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        for (auto& data : reqGLExts) {
            if (!(data.avail || std::strcmp(data.name, extName))) {
                data.avail = true;
                ++availExtsNum;
            }
        }
        std::cout << extName << "\n";
    }
    if (availExtsNum != std::size(reqGLExts)) {
        throw std::runtime_error("Required extensions is not available");
    }

    GLint val;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &val);
    std::cout << "GL_MAX_ARRAY_TEXTURE_LAYERS: " << val;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
    std::cout << "\nGL_MAX_TEXTURE_SIZE: " << val << std::endl;

    context.enableVSync(1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
#ifdef DSE_MULTISAMPLE
    glEnable(GL_MULTISAMPLE);
#endif

    PrepareShaders();
    PrepareSamplers();

    auto size = wnd.Size();
    RebuildViewport(size.x(), size.y());

    glBindVertexArray(vao);
    vbo.bind();
    glBufferData(vbo.target, sizeof(fullscreenPrimitive), fullscreenPrimitive, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glClearColor(.03125f, .03125f, .0625f, 1.f);
}

void RenderOpenGL31_impl::RebuildViewport(unsigned width, unsigned height)
{
    if (this->width == width && this->height == height) return;
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
    glUseProgram(fragmentProg);
    glUniform2f(fragWindowSizeUniform, width, height);
    glUseProgram(drawProg);
    glUniform2f(drawWindowSizeUniform, width, height);

    RebuildSrgbFrameBuffer();
}

void RenderOpenGL31_impl::OnResize(core::WndEvtDt, int width, int height,
        core::WindowShowCommand) {
    RebuildViewport(width, height);
}

void RenderOpenGL31_impl::Render(const util::FunctionPtr<void()>& cb) {
    while (requested.load(std::memory_order_acquire));
    renderCallback = cb;
    requested.store(true, std::memory_order_release);
#ifdef _WIN32
    auto hWnd = wnd->GetSysData().hWnd;
    InvalidateRect(hWnd, nullptr, FALSE);
//    UpdateWindow(hWnd);
#endif
}

void RenderOpenGL31_impl::SetScene(dse::core::Scene &scene) {
    scnChangeCon.unsubscribe();
    this->scene = &scene;
    objects.clear();
}

void RenderOpenGL31_impl::PrepareShaders() {
    glwrp::VertexShader vertShader;
    fragmentProg.attachShader(vertShader);
    vertShader.loadSource(gl31::shader_post_vert);
    vertShader.compile();
    glwrp::FragmentShader fragShader;
    fragmentProg.attachShader(fragShader);
    fragShader.loadSource(gl31::shader_post_frag);
    fragShader.compile();
    glBindAttribLocation(fragmentProg, 0, "pos");
    glBindFragDataLocation(fragmentProg, 0, "fragColor");
    fragmentProg.link();
    fragWindowSizeUniform = glGetUniformLocation(fragmentProg, "windowSize");
    glUseProgram(fragmentProg);
    glUniform2f(fragWindowSizeUniform, width, height);
    glUniform1i(glGetUniformLocation(fragmentProg, "colorBuffer"), TextureUnits::PostProcColor);
    glUniform1i(glGetUniformLocation(fragmentProg, "depthBuffer"), TextureUnits::PostProcDepth);

    vertShader = {};
    drawProg.attachShader(vertShader);
    vertShader.loadSource(gl31::shader_draw_vert);
    vertShader.compile();
    fragShader = {};
    drawProg.attachShader(fragShader);
    fragShader.loadSource(gl31::shader_draw_frag);
    fragShader.compile();
    glBindAttribLocation(drawProg, InputParams::Position, "vPos");
    glBindAttribLocation(drawProg, InputParams::Normal, "vNorm");
    glBindAttribLocation(drawProg, InputParams::Tangent, "vTang");
    glBindAttribLocation(drawProg, InputParams::UV, "vUV");
    glBindAttribLocation(drawProg, InputParams::BTangSign, "vBTangSign");
    glBindFragDataLocation(drawProg, OutputParams::FragmentColor, "fragColor");
    drawProg.link();

    auto objectInstanceBlockIndex = drawProg.getUniformBlockIndex("ObjectInstance");
    glUniformBlockBinding(drawProg, objectInstanceBlockIndex, UniformIndices::ObjectInstanceBind);
    auto cameraBlockIndex = drawProg.getUniformBlockIndex("Camera");
    glUniformBlockBinding(drawProg, cameraBlockIndex, UniformIndices::CameraBind);
    auto materialBlockIndex = drawProg.getUniformBlockIndex("Material");
    glUniformBlockBinding(drawProg, materialBlockIndex, UniformIndices::MaterialBind);
    cameraUBO = {};
    cameraUBO.bind();
    glBufferData(cameraUBO.target, sizeof(CameraUniform), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(cameraUBO.target, UniformIndices::CameraBind, cameraUBO);

    gl31::ObjectMaterialUniform emptyMaterial;
    emptyMaterial.color = {1.f, 0.f, 1.f, 0.f};
    emptyMaterialUBO.bind();
    glBufferData(emptyMaterialUBO.target, sizeof(emptyMaterial), &emptyMaterial, GL_DYNAMIC_DRAW);

    drawWindowSizeUniform = glGetUniformLocation(drawProg, "windowSize");
    glUseProgram(drawProg);
    glUniform2f(drawWindowSizeUniform, 0, 0);
    glUniform1i(glGetUniformLocation(drawProg, "diffuse"), TextureUnits::DrawDiffuse);
    glUniform1i(glGetUniformLocation(drawProg, "normalMap"), TextureUnits::DrawNormalMap);
    pendingTexture = {};
    glTexImage2D(pendingTexture.target, 0, GL_SRGB, 8, 8, 0, GL_BGR, GL_UNSIGNED_BYTE, pendingTextureData.data());
    glGenerateMipmap(pendingTexture.target);
    defaultNormalMap = {};
    glTexImage2D(defaultNormalMap.target, 0, GL_RGBA, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, defaultNormalMapData.data());
}

void RenderOpenGL31_impl::RebuildSrgbFrameBuffer() {
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    renderFBO = {};
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    colorBuffer = {};
    glTexParameteri(colorBuffer.target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(colorBuffer.target, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(colorBuffer.target, 0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment(OutputParams::FragmentColor), colorBuffer.target, colorBuffer, 0);
    depthBuffer = {};
    glTexParameteri(depthBuffer.target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(depthBuffer.target, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(depthBuffer.target, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthBuffer.target, depthBuffer, 0);
#ifdef DSE_MULTISAMPLE
    renderFBOMSAA = {};
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBOMSAA);
    colorBufferMSAA = {};
    glBindRenderbuffer(GL_RENDERBUFFER, colorBufferMSAA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, DSE_MULTISAMPLE, GL_R11F_G11F_B10F, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferMSAA);
    depthBufferMSAA = {};
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferMSAA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, DSE_MULTISAMPLE, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferMSAA);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif
    glFinish();
}

void RenderOpenGL31_impl::SetCamera(dse::core::Camera &camera) {
    this->camera = &camera;
}

auto RenderOpenGL31_impl::GetObjectInstance(core::Object* object) -> gl31::ObjectInstance*
{
    auto it = objects.find(object);
    if (it == objects.end()) {
        auto [it2, result] = objects.emplace(
            std::piecewise_construct,
            std::make_tuple(object),
            std::make_tuple(this, object)
        );
        if (!result) {
            throw std::bad_alloc();
        }
        it = it2;
    }
    return &(it->second);
}

} /* namespace dse::ogl31rbe */
