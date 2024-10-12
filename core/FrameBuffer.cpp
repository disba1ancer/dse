#include <dse/core/FrameBuffer.h>
#include "FrameBuffer_win32.h"

namespace dse::core {

FrameBuffer::FrameBuffer(core::Window& wnd) :
    impl(new FrameBuffer_impl(wnd))
{}

FrameBuffer::~FrameBuffer()
{}

void FrameBuffer::Render(util::function_ptr<void ()> callback)
{
    impl->Render(callback);
}

void FrameBuffer::SetDrawCallback(util::function_ptr<void (void *, math::ivec2)> callback)
{
    impl->SetDrawCallback(callback);
}

} // namespace dse::core
