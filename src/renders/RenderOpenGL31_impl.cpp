/*
 * RenderOpenGLimpl.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include <array>
#include <cstdio>
#include <glbinding/gl31/gl.h>
#include <glbinding/gl31ext/gl.h>
#include "RenderOpenGL31_impl.h"
#include "os/win32.h"
#include "os/WindowData_win32.h"
#include "glwrp/gl.h"
#include "dse_shaders/gl31.h"
#include "gl31/binds.h"
#include "scn/Material.h"
#include "math/qmath.h"
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
using dse::renders::gl31::InputParams;
using dse::renders::gl31::OutputParams;
using dse::renders::gl31::UniformIndices;
using dse::renders::gl31::ObjectInstanceUniform;
using dse::renders::gl31::CameraUniform;
using dse::renders::gl31::TextureUnits;
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
}

//extern "C" __declspec(dllexport) int NvOptimusEnablement = 1;

namespace dse::renders {

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

void dse::renders::RenderOpenGL31_impl::DrawPostprocess() {
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
	glActiveTexture(texture(TextureUnits::PostProcDepth));
	depthBuffer.bind();
	glUseProgram(fragmentProg);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDepthFunc(GL_LESS);
}

void dse::renders::RenderOpenGL31_impl::ReloadInstance(gl31::ObjectInstance& objInst) {
	auto meshInst = objInst.mesh.get();
	if (meshInst == nullptr || meshInst->getMesh() != objInst.object->getMesh()) {
		auto mi = GetMeshInstance(objInst.object->getMesh());
		if (mi) mi->AddRef();
		objInst.mesh.reset(mi);
	}
	ObjectInstanceUniform uniforms;
	auto rotScale = math::transpose(math::matFromQuat(objInst.object->getQRot()));
	auto vPos = objInst.object->getPos();
	auto vScale = objInst.object->getScale();
	rotScale[0] *= vScale;
	rotScale[1] *= vScale;
	rotScale[2] *= vScale;
	uniforms.transform = { math::vec4
		{rotScale[0].x(), rotScale[0].y(), rotScale[0].z(), vPos.x()},
		{rotScale[1].x(), rotScale[1].y(), rotScale[1].z(), vPos.y()},
		{rotScale[2].x(), rotScale[2].y(), rotScale[2].z(), vPos.z()}
	};
	if (objInst.ubo == 0) {
		objInst.ubo = {};
		glBufferData(objInst.ubo.target, sizeof(uniforms), nullptr, GL_DYNAMIC_DRAW);
	}
	objInst.ubo.bind();
	glBufferSubData(objInst.ubo.target, 0, sizeof(uniforms), &uniforms);
	objInst.lastVersion = objInst.object->getVersion();
}

void RenderOpenGL31_impl::OnSceneChanged(scn::SceneChangeEventType act, scn::Object* obj) {
	switch (act) {
		case decltype(act)::ObjectCreate: {
			auto& inst = objects[obj];
			inst.object = obj;
			inst.lastVersion = obj->getVersion() - 1;
		} break;
		case decltype(act)::ObjectDestroy: {
			objects.erase(obj);
		} break;
	}
}

void RenderOpenGL31_impl::CleanupMeshes() {
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

void RenderOpenGL31_impl::FillInstances() {
	for (auto& object : (scene->objects())) {
		auto& objInst = objects[&object];
		objInst.object = &object;
		ReloadInstance(objInst);
	}
	scnChangeCon = scene->subscribeChangeEvent(util::StaticMemFn<&RenderOpenGL31_impl::OnSceneChanged>(*this));
}

auto RenderOpenGL31_impl::GetMeshInstance(scn::IMesh* mesh) -> gl31::MeshInstance* {
	if (mesh) {
		auto i = meshes.find(mesh);
		if (i == meshes.end()) {
			auto [i, r] = meshes.emplace(
				std::piecewise_construct,
				std::make_tuple(mesh),
				std::make_tuple(mesh)
			);
			if (!r) {
				throw std::bad_alloc();
			} else {
				return &(i->second);
			}
		}
		return &(i->second);
	}
	return nullptr;
}

void RenderOpenGL31_impl::OnPaint(os::WndEvtDt) {
#ifdef DSE_MULTISAMPLE
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBOMSAA);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindSampler(TextureUnits::DrawDiffuse, drawDiffuse);
	glActiveTexture(texture(TextureUnits::DrawDiffuse));
	pendingTexture.bind();
	glUseProgram(drawProg);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (camera) {
		SetupCamera();
		if (scene) {
			if (objects.empty()) {
				FillInstances();
			}
			for (auto& [obj, inst] : objects) {
				if (inst.lastVersion != obj->getVersion()) {
					ReloadInstance(inst);
				}
				if (inst.mesh) {
					auto& mesh = inst.mesh;
					if (mesh) {
						if (mesh->isReady()) {
							glBindVertexArray(mesh->getVAO());
							glBindBufferBase(inst.ubo.target, UniformIndices::ObjectInstanceBind, inst.ubo);
							auto subCount = mesh->getSubmeshCount();
							for (std::size_t i = 0; i < subCount; ++i) {
								auto [start, count] = mesh->getSubmeshRange(i);
								auto material = obj->getMaterial(i);
								if (material) {
									glUniform4fv(matColorUnifrom, 1, (material->getColor()).elements);
								} else {
									glUniform4f(matColorUnifrom, 1.f, 0.f, 1.f, 0.f);
								}
								glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(start));
							}
						}
					}
				}
			}
		}
	}
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

void dse::renders::RenderOpenGL31_impl::PrepareSamplers() {
	postProcColor = {};
	postProcDepth = {};
	drawDiffuse = {};
}

RenderOpenGL31_impl::RenderOpenGL31_impl(os::Window& wnd) : wnd(&wnd),
		paintCon(wnd.SubscribePaintEvent(util::StaticMemFn<&RenderOpenGL31_impl::OnPaint>(*this))),
		sizeCon(wnd.SubscribeResizeEvent(util::StaticMemFn<&RenderOpenGL31_impl::OnResize>(*this))),
		context(wnd, glwrp::ContextVersion::gl31, glwrp::ContextFlags::Debug),
		cleanupPointer(meshes.end())
{
	GLint numExts;
	unsigned availExtsNum = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
	for (int i = 0; i < numExts; ++i) {
		auto extName = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
		for (auto& data : reqGLExts) {
			if (!(data.avail || strcmp(data.name, extName))) {
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

//	context.enableVSync(1);
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

void RenderOpenGL31_impl::OnResize(os::WndEvtDt, int width, int height,
		os::WindowShowCommand) {
	RebuildViewport(width, height);
}

void RenderOpenGL31_impl::Render(const util::FunctionPtr<void()>& cb) {
	while (requested.load(std::memory_order_acquire));
	renderCallback = cb;
	requested.store(true, std::memory_order_release);
#ifdef _WIN32
	auto hWnd = wnd->GetSysData().hWnd;
	InvalidateRect(hWnd, nullptr, FALSE);
	//UpdateWindow(hWnd);
#endif
}

void RenderOpenGL31_impl::SetScene(dse::scn::Scene &scene) {
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
	glBindFragDataLocation(drawProg, OutputParams::FragmentColor, "fragColor");
	drawProg.link();

	auto objectInstanceBlockIndex = drawProg.getUniformBlockIndex("ObjectInstance");
	glUniformBlockBinding(drawProg, objectInstanceBlockIndex, UniformIndices::ObjectInstanceBind);
	auto cameraBlockIndex = drawProg.getUniformBlockIndex("Camera");
	glUniformBlockBinding(drawProg, cameraBlockIndex, UniformIndices::CameraBind);
	cameraUBO = {};
	cameraUBO.bind();
	glBufferData(cameraUBO.target, sizeof(CameraUniform), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(cameraUBO.target, UniformIndices::CameraBind, cameraUBO);

	drawWindowSizeUniform = glGetUniformLocation(drawProg, "windowSize");
	matColorUnifrom = glGetUniformLocation(drawProg, "matColor");
	glUseProgram(drawProg);
	glUniform2f(drawWindowSizeUniform, 0, 0);
	glUniform1i(glGetUniformLocation(drawProg, "diffuse"), TextureUnits::DrawDiffuse);
	pendingTexture = {};
	glTexImage2D(pendingTexture.target, 0, GL_SRGB, 8, 8, 0, GL_BGR, GL_UNSIGNED_BYTE, pendingTextureData.data());
	glGenerateMipmap(pendingTexture.target);
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

void RenderOpenGL31_impl::SetCamera(dse::scn::Camera &camera) {
	this->camera = &camera;
}

} /* namespace dse::renders */
