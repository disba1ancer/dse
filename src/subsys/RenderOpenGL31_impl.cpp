/*
 * RenderOpenGLimpl.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "os/win32.h"
#include <swal/window.h>
#include "os/WindowData_win32.h"
#include "RenderOpenGL31_impl.h"
#include "gl/gl.h"
#include "gl31_impl/shaders.h"
#include "gl31_impl/binds.h"
#include <array>
#include <cstdio>
#include "scn/Material.h"

namespace {
struct ReqGLExt {
	const char* name;
	bool avail;
};
auto reqGLExts = std::to_array<ReqGLExt, 2>({
	{"GL_ARB_draw_elements_base_vertex", false},
	{"GL_ARB_instanced_arrays", false},
});
using dse::threadutils::ExecutionThread;
using dse::subsys::gl31_impl::InputParams;
using dse::subsys::gl31_impl::OutputParams;
dse::math::vec3 fullscreenPrimitive[] = { {-1.f, -1.f, -1.f}, {3.f, -1.f, -1.f}, {-1.f, 3.f, -1.f} };
}

namespace dse {
namespace subsys {

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
		glUniform3fv(camPosUniform, 1, camera->getPos().elements);
		glUniform4fv(camQRotUniform, 1, camera->getRot().elements);
		auto zNear = camera->getNear(),
				zFar = camera->getFar();
		if (zNear == zFar) {
			zNear = 1.f;
			zFar = 131072.f;
		}
		float c = -1.f / camera->getFocalLength();
		float a = c / (zFar - zNear);
		float b = 2 * zFar * zNear * a;
		a *= (zFar+ zNear);
		glUniform4f(perspArgsUniform, a, b, c, 0.f);
		if (scene) {
			for (scn::Object& object : (scene->objects())) {
				if (object.getMesh()) {
					auto mesh = reinterpret_cast<gl31_impl::MeshInstance*>(object.getMesh()->getCustomValue(this));
					if (!mesh) {
						mesh = &*meshes.emplace(meshes.end(), object.getMesh());
						mesh->getMesh()->storeCustomValue(this, mesh);
					}
					if (mesh->isReady()) {
						glBindVertexArray(mesh->getVAO());
						glUniform3fv(posUniform, 1, (object.getPos()).elements);
						glUniform4fv(qRotUniform, 1, (object.getQRot()).elements);
						glUniform3fv(scaleUniform, 1, (object.getScale()).elements);
						auto subCount = mesh->getSubmeshCount();
						for (std::size_t i = 0; i < subCount; ++i) {
							auto [start, count] = mesh->getSubmeshRange(i);
							auto material = object.getMaterial(i);
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
	context.SwapBuffers();
	swal::Wnd(wnd->getSysData().hWnd).ValidateRect();
	threadutils::ThreadPool::Task* t;
	if ((t = task.exchange(nullptr, std::memory_order_acquire))) {
		pool.schedule(*t);
	}
}

RenderOpenGL31_impl::RenderOpenGL31_impl(os::Window& wnd) : wnd(&wnd),
		paintCon(wnd.subscribePaintEvent(notifier::make_handler<&RenderOpenGL31_impl::onPaint>(this))),
		sizeCon(wnd.subscribeResizeEvent(notifier::make_handler<&RenderOpenGL31_impl::onResize>(this))),
		context(wnd)
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

	context.enableVSync(0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
#ifdef DSE_MULTISAMPLE
	glEnable(GL_MULTISAMPLE);
#endif

	prepareShaders();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenPrimitive), fullscreenPrimitive, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glClearColor(.03125f, .03125f, .0625f, 1.f);
}

void RenderOpenGL31_impl::onResize(os::WndEvtDt, int width, int height,
		os::WindowShowCommand) {
	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);
	glUseProgram(fragmentProg);
	glUniform2f(fragWindowSizeUniform, width, height);
	glUseProgram(drawProg);
	glUniform2f(drawWindowSizeUniform, width, height);
	glUniform1f(invAspRatioUniform, float(height) / float(width));

	rebuildSrgbFrameBuffer();
}

threadutils::TaskState RenderOpenGL31_impl::renderTask() {
	if (renderingTaskStage == 0) {
		auto ptr = threadutils::ThreadPool::getCurrentPool();
		pool = ptr;
		renderingTaskStage = 1;
		task.store(threadutils::ThreadPool::getCurrentTask(), std::memory_order_release);
#ifdef _WIN32
		auto hWnd = wnd->getSysData().hWnd;
		InvalidateRect(hWnd, nullptr, FALSE);
		//UpdateWindow(hWnd);
#endif
		return threadutils::TaskState::Await;
	}
	renderingTaskStage = 0;
	return threadutils::TaskState::End;
}

void RenderOpenGL31_impl::setScene(dse::scn::Scene &scene) {
	this->scene = &scene;
}

void RenderOpenGL31_impl::prepareShaders() {
	gl::VertexShader vertShader;
	fragmentProg.attachShader(vertShader);
	vertShader.loadSource(gl31_impl::fbVertexShader);
	vertShader.compile();
	gl::FragmentShader fragShader;
	fragmentProg.attachShader(fragShader);
	fragShader.loadSource(gl31_impl::fbFragmentShader);
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

	vertShader = gl::VertexShader();
	drawProg.attachShader(vertShader);
	vertShader.loadSource(gl31_impl::drawVertexShader);
	vertShader.compile();
	fragShader = gl::FragmentShader();
	drawProg.attachShader(fragShader);
	fragShader.loadSource(gl31_impl::drawFragmentShader);
	fragShader.compile();
	glBindAttribLocation(drawProg, InputParams::Position, "vPos");
	glBindAttribLocation(drawProg, InputParams::Normal, "vNorm");
	glBindAttribLocation(drawProg, InputParams::Tangent, "vTang");
	glBindAttribLocation(drawProg, InputParams::UV, "vUV");
	glBindFragDataLocation(drawProg, OutputParams::FragmentColor, "fragColor");
	drawProg.link();
	drawWindowSizeUniform = glGetUniformLocation(drawProg, "windowSize");
	posUniform = glGetUniformLocation(drawProg, "iPos");
	qRotUniform = glGetUniformLocation(drawProg, "qRot");
	scaleUniform = glGetUniformLocation(drawProg, "scale");
	camPosUniform = glGetUniformLocation(drawProg, "camPos");
	camQRotUniform = glGetUniformLocation(drawProg, "camQRot");
	matColorUnifrom = glGetUniformLocation(drawProg, "matColor");
	invAspRatioUniform = glGetUniformLocation(drawProg, "invAspRatio");
	perspArgsUniform = glGetUniformLocation(drawProg, "perspArgs");
	glUseProgram(drawProg);
	glUniform2f(drawWindowSizeUniform, 0, 0);
	glUniform1f(invAspRatioUniform, 1.f);
}

void RenderOpenGL31_impl::rebuildSrgbFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderFBO = gl::FrameBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	colorBuffer = gl::TextureRectangle();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorBuffer, 0);
	depthBuffer = gl::TextureRectangle();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, depthBuffer, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
#ifdef DSE_MULTISAMPLE
	renderFBOMSAA = gl::FrameBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBOMSAA);
	colorBufferMSAA = gl::RenderBuffer();
	glBindRenderbuffer(GL_RENDERBUFFER, colorBufferMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, DSE_MULTISAMPLE, GL_R11F_G11F_B10F, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferMSAA);
	depthBufferMSAA = gl::RenderBuffer();
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, DSE_MULTISAMPLE, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif
}

void RenderOpenGL31_impl::setCamera(dse::scn::Camera &camera) {
	this->camera = &camera;
}

} /* namespace subsys */
} /* namespace dse */
