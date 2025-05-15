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

class API_DSE_CORE FrameBuffer
{
public:
    FrameBuffer(core::Window& wnd);
    ~FrameBuffer();
    void Render();
    void SetDrawCallback(util::function_ptr<void(void*, math::ivec2)> callback);
private:
    std::unique_ptr<FrameBuffer_impl> impl;
};

} // namespace dse::core

#endif // DSE_CORE_FRAMEBUFFER_H
