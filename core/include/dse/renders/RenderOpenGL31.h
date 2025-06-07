/*
 * RenderOpenGL.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_RENDERS_RENDEROPENGL_H_
#define DSE_RENDERS_RENDEROPENGL_H_

#include <memory>
#include <dse/core/Window.h>
#include <dse/core/scene2.h>
#include <dse/core/Camera.h>
#include <dse/util/functional.h>
#include <dse/util/execution.h>
#include <dse/core/detail/impexp.h>

#ifdef DSE_OGL31VIDEO_EXPORT
#define API_DSE_OGL31VIDEO API_EXPORT_DSE
#else
#define API_DSE_OGL31VIDEO API_IMPORT_DSE
#endif

namespace dse::ogl31rbe {

class RenderOpenGL31_impl;

class API_DSE_OGL31VIDEO RenderOpenGL31 {
	std::unique_ptr<RenderOpenGL31_impl> impl;
public:
	RenderOpenGL31(core::Window& wnd);
	~RenderOpenGL31();
	RenderOpenGL31(const RenderOpenGL31&) = delete;
	RenderOpenGL31(RenderOpenGL31&&) = delete;
	auto operator=(const RenderOpenGL31&) -> RenderOpenGL31& = delete;
	auto operator=(RenderOpenGL31&&) -> RenderOpenGL31& = delete;
    void Render();
    void SetScene(dse::core::IScene* scene);
	void SetCamera(dse::core::Camera& camera);
};

} /* namespace dse::renders */

#endif /* DSE_RENDERS_RENDEROPENGL_H_ */
