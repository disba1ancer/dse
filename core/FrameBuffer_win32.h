#ifndef DSE_CORE_FRAMEBUFFER_WIN32_H
#define DSE_CORE_FRAMEBUFFER_WIN32_H

#include <dse/core/Image.h>
#include <dse/core/Window.h>
#include <dse/util/functional.h>
#include <swal/gdi.h>

namespace dse::core {

class FrameBuffer_win32
{
public:
    FrameBuffer_win32(core::Window& wnd);
    FrameBuffer_win32(const FrameBuffer_win32&) = delete;
    FrameBuffer_win32(FrameBuffer_win32&&) = delete;
    FrameBuffer_win32& operator=(const FrameBuffer_win32&) = delete;
    FrameBuffer_win32& operator=(FrameBuffer_win32&&) = delete;
    ~FrameBuffer_win32();
    void Render(util::function_ptr<void()> callback);
    void SetDrawCallback(util::function_ptr<void(void*, math::ivec2)> callback);
private:
    void OnPaint(core::WndEvtDt);
    void OnResize(core::WndEvtDt, int w, int h, WindowShowCommand);
    void DrawDC(const swal::DC& dc, const RECT& rc);

    core::Window& window;
    std::atomic_flag sync;
    util::function_ptr<void (void*, math::ivec2)> renderCallback = nullptr;
    util::function_ptr<void ()> exitCallback = nullptr;
    Image frameBuffer{window.Size()};
    notifier::connection<core::Window::PaintHandler> paintCon;
    notifier::connection<core::Window::ResizeHandler> resizeCon;
};

} // namespace dse::core

#endif // DSE_CORE_FRAMEBUFFER_WIN32_H
