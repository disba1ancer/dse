#ifndef DSE_CORE_FRAMEBUFFER_H
#define DSE_CORE_FRAMEBUFFER_H

#include "Window.h"
#include <memory>
#include <dse/util/execution.h>
#include <dse/util/functional.h>

namespace dse::core {

#ifdef _WIN32

class FrameBuffer_win32;

typedef FrameBuffer_win32 FrameBuffer_impl;

#endif

namespace fbimpl {

class RenderSender;

}

class API_DSE_CORE FrameBuffer
{
public:
    FrameBuffer(core::Window& wnd);
    ~FrameBuffer();
    void Render(util::function_ptr<void()> callback);
    auto Render() -> fbimpl::RenderSender;
    void SetDrawCallback(util::function_ptr<void(void*, math::ivec2)> callback);
private:
    std::unique_ptr<FrameBuffer_impl> impl;
};

namespace fbimpl {

template <typename Recv>
class RenderOpstate {
	FrameBuffer& render;
	std::remove_cvref_t<Recv> receiver;
	void EndCallback()
	{
		util::SetValue(std::move(receiver));
	}
public:
	RenderOpstate(FrameBuffer& render, Recv&& recv) :
		render(render),
		receiver(std::forward<Recv>(recv))
	{}
	friend void dse_TagInvoke(util::TagT<util::Start>, RenderOpstate& opstate)
	{
		opstate.render.Render({opstate, util::fn_tag<&RenderOpstate::EndCallback>});
	}
};

class RenderSender {
	FrameBuffer& render;
public:
	template <template<typename ...> typename Tuple>
	using ValueTypes = Tuple<>;
	using ErrorType = std::exception_ptr;
	static constexpr bool SendsDone = false;
	RenderSender(FrameBuffer& render) : render(render)
	{}
	template <typename Recv>
	friend auto dse_TagInvoke(util::TagT<util::Connect>, RenderSender&& sndr, Recv&& recv)
	-> RenderOpstate<Recv>
	{
		return { sndr.render, std::forward<Recv>(recv) };
	}
};

}

inline auto API_DSE_CORE FrameBuffer::Render()
-> fbimpl::RenderSender
{
	return { *this };
}

} // namespace dse::core

#endif // DSE_CORE_FRAMEBUFFER_H
