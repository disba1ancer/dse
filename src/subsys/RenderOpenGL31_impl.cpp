/*
 * RenderOpenGLimpl.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "../os/WindowData_win32.h"
#include "RenderOpenGL31_impl.h"
#include "gl/gl.h"
#include "gl31_impl/shaders.h"
#include "gl31_impl/binds.h"

namespace {
struct ReqGLExt {
	const char* name;
	bool avail;
};
ReqGLExt reqGLExts[] = {
		{"GL_ARB_draw_elements_base_vertex", false},
		{"GL_ARB_instanced_arrays", false},
};
using dse::threadutils::ExecutionThread;
using dse::subsys::gl31_impl::InputParams;
using dse::subsys::gl31_impl::OutputParams;
dse::math::vec3 fullscreenPrimitive[] = { {-1.f, -1.f, -1.f}, {-1.f, 3.f, -1.f}, {3.f, -1.f, -1.f} };
}

namespace dse {
namespace subsys {

void RenderOpenGL31_impl::onPaint(os::WndEvtDt, os::PntEvtDt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(drawProg);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
						glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(start));
					}
				}
			}
		}
	}
	glUseProgram(fragmentProg);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	context.SwapBuffers();
}

RenderOpenGL31_impl::RenderOpenGL31_impl(os::Window& wnd) : wnd(&wnd),
		paintCon(wnd.subscribePaintEvent(notifier::make_handler<&onPaint>(this))),
		sizeCon(wnd.subscribeResizeEvent(notifier::make_handler<&onResize>(this))),
		context(wnd),
		scene(nullptr),
		windowSizeUniform(0),
		posUniform(0),
		qRotUniform(0),
		scaleUniform(0) {
	GLint numExts;
	int availExtsNum = 0;
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
	if (availExtsNum != sizeof(reqGLExts) / sizeof(reqGLExts[0])) {
		throw std::runtime_error("Required extensions is not available");
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);
	glClearDepth(.0);

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
	windowSizeUniform = glGetUniformLocation(fragmentProg, "windowSize");
	glUseProgram(fragmentProg);
	glUniform2f(windowSizeUniform, 0, 0);

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
	posUniform = glGetUniformLocation(drawProg, "iPos");
	qRotUniform = glGetUniformLocation(drawProg, "qRot");
	scaleUniform = glGetUniformLocation(drawProg, "scale");

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenPrimitive), fullscreenPrimitive, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glClearColor(.0f, .0f, .0f, .0f);
}

void RenderOpenGL31_impl::onResize(os::WndEvtDt, int width, int height,
		os::WindowShowCommand) {
	glViewport(0, 0, width, height);
	glUniform2f(windowSizeUniform, width, height);
}

bool RenderOpenGL31_impl::renderTask() {
#ifdef _WIN32
	auto hWnd = wnd->getSysData().hWnd;
	InvalidateRect(hWnd, nullptr, FALSE);
	//UpdateWindow(hWnd);
#endif
	return true;
}

void RenderOpenGL31_impl::setScene(dse::scn::Scene &scene) {
	this->scene = &scene;
}

} /* namespace subsys */
} /* namespace dse */
