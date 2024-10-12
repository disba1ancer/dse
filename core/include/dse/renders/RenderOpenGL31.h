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
#include <dse/core/Scene.h>
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

namespace oglimpl {

class RenderSender;

}

class API_DSE_OGL31VIDEO RenderOpenGL31 {
	std::unique_ptr<RenderOpenGL31_impl> impl;
public:
	RenderOpenGL31(core::Window& wnd);
	~RenderOpenGL31();
	RenderOpenGL31(const RenderOpenGL31&) = delete;
	RenderOpenGL31(RenderOpenGL31&&) = delete;
	auto operator=(const RenderOpenGL31&) -> RenderOpenGL31& = delete;
	auto operator=(RenderOpenGL31&&) -> RenderOpenGL31& = delete;
	void Render(const util::FunctionPtr<void()>& cb);
	auto Render() -> oglimpl::RenderSender;
	void SetScene(dse::core::Scene& scene);
	void SetCamera(dse::core::Camera& camera);
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
		opstate.render.Render({opstate, util::fn_tag<&RenderOpstate::EndCallback>});
	}
};

class RenderSender {
	RenderOpenGL31& render;
public:
	template <template<typename ...> typename Tuple>
	using ValueTypes = Tuple<>;
	using ErrorType = std::exception_ptr;
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

inline auto API_DSE_OGL31VIDEO RenderOpenGL31::Render()
-> oglimpl::RenderSender
{
	return { *this };
}

} /* namespace dse::renders */

#endif /* DSE_RENDERS_RENDEROPENGL_H_ */
