/*
 * RenderOpenGL.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_RENDEROPENGL_H_
#define SUBSYS_RENDEROPENGL_H_

#include <memory>
#include "os/Window.h"
#include "scn/Scene.h"
#include "scn/Camera.h"
#include "util/future.h"

namespace dse {
namespace subsys {

class RenderOpenGL31_impl;

class RenderOpenGL31 {
	std::unique_ptr<RenderOpenGL31_impl> impl;
public:
	RenderOpenGL31(os::Window& wnd);
	~RenderOpenGL31();
	RenderOpenGL31(const RenderOpenGL31&) = delete;
	RenderOpenGL31(RenderOpenGL31&&) = delete;
	auto operator=(const RenderOpenGL31&) -> RenderOpenGL31& = delete;
	auto operator=(RenderOpenGL31&&) -> RenderOpenGL31& = delete;
	auto render() -> util::future<void>;
	void setScene(dse::scn::Scene& scene);
	void setCamera(dse::scn::Camera& camera);
};

} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_RENDEROPENGL_H_ */
