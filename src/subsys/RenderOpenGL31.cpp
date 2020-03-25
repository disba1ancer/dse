/*
 * RenderOpenGL.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "RenderOpenGL31_impl.h"
#include "RenderOpenGL31.h"

namespace dse {
namespace subsys {

RenderOpenGL31::RenderOpenGL31(os::Window& wnd) : impl(new RenderOpenGL31_impl(wnd)) {
}

RenderOpenGL31::~RenderOpenGL31() = default;

bool RenderOpenGL31::renderTask() {
	return impl->renderTask();
}

void RenderOpenGL31::setScene(dse::scn::Scene &scene) {
	impl->setScene(scene);
}

void RenderOpenGL31::setCamera(dse::scn::Camera &camera) {
	impl->setCamera(camera);
}

} /* namespace subsys */
} /* namespace dse */
