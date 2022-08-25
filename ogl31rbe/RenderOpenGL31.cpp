/*
 * RenderOpenGL.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "RenderOpenGL31_impl.h"
#include <dse/renders/RenderOpenGL31.h>

namespace dse::ogl31rbe {

RenderOpenGL31::RenderOpenGL31(core::Window& wnd) : impl(new RenderOpenGL31_impl(wnd)) {
}

RenderOpenGL31::~RenderOpenGL31() = default;

void RenderOpenGL31::Render(const util::FunctionPtr<void()>& cb) {
	return impl->Render(cb);
}

void RenderOpenGL31::SetScene(core::Scene &scene) {
	impl->SetScene(scene);
}

void RenderOpenGL31::SetCamera(core::Camera &camera) {
	impl->SetCamera(camera);
}

} /* namespace dse::ogl31rbe */
