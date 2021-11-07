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
#include "util/functional.h"
#include "util/execution.h"

namespace dse::renders {

class RenderOpenGL31_impl;

namespace oglimpl {

class RenderSender;

}

class RenderOpenGL31 {
	std::unique_ptr<RenderOpenGL31_impl> impl;
public:
	RenderOpenGL31(os::Window& wnd);
	~RenderOpenGL31();
	RenderOpenGL31(const RenderOpenGL31&) = delete;
	RenderOpenGL31(RenderOpenGL31&&) = delete;
	auto operator=(const RenderOpenGL31&) -> RenderOpenGL31& = delete;
	auto operator=(RenderOpenGL31&&) -> RenderOpenGL31& = delete;
	void Render(const util::function_view<void()>& cb);
	auto Render() -> oglimpl::RenderSender;
	void SetScene(dse::scn::Scene& scene);
	void SetCamera(dse::scn::Camera& camera);
};

namespace oglimpl {

template <typename Recv>
class RenderOpstate {
	RenderOpenGL31& render;
	std::remove_cvref_t<Recv> receiver;
	void EndCallback()
	{
		util::SetValue(std::move(receiver));
	}
public:
	RenderOpstate(RenderOpenGL31& render, Recv&& recv) :
		render(render),
		receiver(std::forward<Recv>(recv))
	{}
	friend void dse_TagInvoke(util::TagT<util::Start>, RenderOpstate& opstate)
	{
		opstate.render.Render(util::from_method<&RenderOpstate::EndCallback>(opstate));
	}
};

class RenderSender {
	RenderOpenGL31& render;
public:
	template <template<typename ...> typename Tuple>
	using ValueTypes = Tuple<>;
	template <template<typename ...> typename Tuple>
	using ErrorTypes = Tuple<std::exception_ptr>;
	static constexpr bool SendsDone = false;
	RenderSender(RenderOpenGL31& render) : render(render)
	{}
	template <typename Recv>
	friend auto dse_TagInvoke(util::TagT<util::Connect>, RenderSender&& sndr, Recv&& recv)
	-> RenderOpstate<Recv>
	{
		return { sndr.render, std::forward<Recv>(recv) };
	}
};

}

inline auto RenderOpenGL31::Render()
-> oglimpl::RenderSender
{
	return { *this };
}

} /* namespace dse::renders */

#endif /* SUBSYS_RENDEROPENGL_H_ */
