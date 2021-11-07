/*
 * RenderOpenGL.cpp
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "RenderOpenGL31_impl.h"
#include "RenderOpenGL31.h"

namespace dse::renders {

RenderOpenGL31::RenderOpenGL31(os::Window& wnd) : impl(new RenderOpenGL31_impl(wnd)) {
}

RenderOpenGL31::~RenderOpenGL31() = default;

void RenderOpenGL31::Render(const util::function_view<void()>& cb) {
	return impl->Render(cb);
}

void RenderOpenGL31::SetScene(dse::scn::Scene &scene) {
	impl->SetScene(scene);
}

void RenderOpenGL31::SetCamera(dse::scn::Camera &camera) {
	impl->SetCamera(camera);
}

} /* namespace dse::renders */
