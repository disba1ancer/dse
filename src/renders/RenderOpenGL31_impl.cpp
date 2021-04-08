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
using namespace gl31;
using namespace gl31ext;
dse::math::vec3 fullscreenPrimitive[] = { {-1.f, -1.f, -1.f}, {3.f, -1.f, -1.f}, {-1.f, 3.f, -1.f} };
}

namespace dse::renders {

void RenderOpenGL31_impl::setupCamera() {
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
//	glUniform3fv(camPosUniform, 1, camera->getPos().elements);
//	glUniform4fv(camQRotUniform, 1, camera->getRot().elements);
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
//	glUniform4f(perspArgsUniform, a, b, c, 0.f);
	cameraUBO.bind();
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniform), &uniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, UniformIndices::CameraBind, cameraUBO);
}

void dse::renders::RenderOpenGL31_impl::drawPostprocess() {
#ifdef DSE_MULTISAMPLE
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderFBO);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDepthFunc(GL_ALWAYS);
	glActiveTexture(GL_TEXTURE1);
	colorBuffer.bind();
	glActiveTexture(GL_TEXTURE2);
	depthBuffer.bind();
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(fragmentProg);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDepthFunc(GL_LESS);
}

void dse::renders::RenderOpenGL31_impl::reloadInstance(gl31::ObjectInstance& objInst) {
	auto meshInst = objInst.mesh.get();
	if (meshInst == nullptr || meshInst->getMesh() != objInst.object->getMesh()) {
		auto mi = getMeshInstance(objInst.object->getMesh());
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
		objInst.ubo = glwrp::UniformBuffer();
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms), nullptr, GL_DYNAMIC_DRAW);
	}
	objInst.ubo.bind();
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uniforms), &uniforms);
	objInst.lastVersion = objInst.object->getVersion();
}

void RenderOpenGL31_impl::onSceneChanged(scn::SceneChangeEventType act, scn::Object* obj) {
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

void RenderOpenGL31_impl::cleanupMeshes() {
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

void RenderOpenGL31_impl::fillInstances() {
	for (auto& object : (scene->objects())) {
		auto& objInst = objects[&object];
		objInst.object = &object;
		reloadInstance(objInst);
	}
	scnChangeCon = scene->subscribeChangeEvent(util::from_method<&RenderOpenGL31_impl::onSceneChanged>(*this));
}

auto RenderOpenGL31_impl::getMeshInstance(scn::IMesh* mesh) -> gl31::MeshInstance* {
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

void RenderOpenGL31_impl::onPaint(os::WndEvtDt) {
#ifdef DSE_MULTISAMPLE
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBOMSAA);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(drawProg);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (camera) {
		setupCamera();
		if (scene) {
			if (objects.empty()) {
				fillInstances();
			}
			for (auto& [obj, inst] : objects) {
				if (inst.lastVersion != obj->getVersion()) {
					reloadInstance(inst);
				}
				if (inst.mesh) {
					auto& mesh = inst.mesh;
					if (mesh) {
						if (mesh->isReady()) {
							glBindVertexArray(mesh->getVAO());
//							auto v = obj->getPos();
//							glUniform4fv(posUniform, 1, math::vec4{v[0], v[1], v[2], 0}.elements);
//							glUniform4fv(qRotUniform, 1, (obj->getQRot()).elements);
//							v = obj->getScale();
//							glUniform4fv(scaleUniform, 1, math::vec4{v[0], v[1], v[2], 0}.elements);
							glBindBufferBase(GL_UNIFORM_BUFFER, UniformIndices::ObjectInstanceBind, inst.ubo);
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
	drawPostprocess();
	context.SwapBuffers();
	cleanupMeshes();
	swal::Wnd(wnd->getSysData().hWnd).ValidateRect();
	if (requested.exchange(false, std::memory_order_relaxed)) {
		auto pool = core::ThreadPool::getCurrentPool();
		pool.schedule(
			util::from_method<
				&RenderOpenGL31_impl::resumeRenderCaller
			>(*this)
		);
	}
}

RenderOpenGL31_impl::RenderOpenGL31_impl(os::Window& wnd) : wnd(&wnd),
		paintCon(wnd.subscribePaintEvent(util::from_method<&RenderOpenGL31_impl::onPaint>(*this))),
		sizeCon(wnd.subscribeResizeEvent(util::from_method<&RenderOpenGL31_impl::onResize>(*this))),
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
	}
	if (availExtsNum != std::size(reqGLExts)) {
		throw std::runtime_error("Required extensions is not available");
	}

	context.enableVSync(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
#ifdef DSE_MULTISAMPLE
	glEnable(GL_MULTISAMPLE);
#endif

	prepareShaders();

	auto size = wnd.size();
	rebuildViewport(size.x(), size.y());

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenPrimitive), fullscreenPrimitive, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glClearColor(.03125f, .03125f, .0625f, 1.f);
}

void RenderOpenGL31_impl::rebuildViewport(int width, int height)
{
	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);
	glUseProgram(fragmentProg);
	glUniform2f(fragWindowSizeUniform, width, height);
	glUseProgram(drawProg);
	glUniform2f(drawWindowSizeUniform, width, height);
//	glUniform1f(invAspRatioUniform, float(height) / float(width));

	rebuildSrgbFrameBuffer();
}

void RenderOpenGL31_impl::onResize(os::WndEvtDt, int width, int height,
		os::WindowShowCommand) {
	rebuildViewport(width, height);
}

auto RenderOpenGL31_impl::render() -> util::future<void> {
	pr = util::promise<void>();
	requested.store(true, std::memory_order_relaxed);
#ifdef _WIN32
	auto hWnd = wnd->getSysData().hWnd;
	InvalidateRect(hWnd, nullptr, FALSE);
	//UpdateWindow(hWnd);
#endif
	return pr.get_future();
}

void RenderOpenGL31_impl::setScene(dse::scn::Scene &scene) {
	scnChangeCon.unsubscribe();
	this->scene = &scene;
	objects.clear();
}

void RenderOpenGL31_impl::prepareShaders() {
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
	fragColorBufferUniform = glGetUniformLocation(fragmentProg, "colorBuffer");
	fragDepthBufferUniform = glGetUniformLocation(fragmentProg, "depthBuffer");
	glUseProgram(fragmentProg);
	glUniform2f(fragWindowSizeUniform, width, height);
	glUniform1i(fragColorBufferUniform, 1);
	glUniform1i(fragDepthBufferUniform, 2);

	vertShader = glwrp::VertexShader();
	drawProg.attachShader(vertShader);
	vertShader.loadSource(gl31::shader_draw_vert);
	vertShader.compile();
	fragShader = glwrp::FragmentShader();
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
	cameraUBO = glwrp::UniformBuffer();
	cameraUBO.bind();
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUniform), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, UniformIndices::CameraBind, cameraUBO);

	drawWindowSizeUniform = glGetUniformLocation(drawProg, "windowSize");
	matColorUnifrom = glGetUniformLocation(drawProg, "matColor");
	glUseProgram(drawProg);
	glUniform2f(drawWindowSizeUniform, 0, 0);
}

void RenderOpenGL31_impl::resumeRenderCaller() {
	pr.set_value();
}

void RenderOpenGL31_impl::rebuildSrgbFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderFBO = glwrp::FrameBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	colorBuffer = glwrp::TextureRectangle();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorBuffer, 0);
	depthBuffer = glwrp::TextureRectangle();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, depthBuffer, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
#ifdef DSE_MULTISAMPLE
	renderFBOMSAA = glwrp::FrameBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBOMSAA);
	colorBufferMSAA = glwrp::RenderBuffer();
	glBindRenderbuffer(GL_RENDERBUFFER, colorBufferMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, DSE_MULTISAMPLE, GL_R11F_G11F_B10F, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferMSAA);
	depthBufferMSAA = glwrp::RenderBuffer();
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, DSE_MULTISAMPLE, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif
}

void RenderOpenGL31_impl::setCamera(dse::scn::Camera &camera) {
	this->camera = &camera;
}

} /* namespace dse::renders */
