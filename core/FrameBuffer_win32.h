#ifndef DSE_CORE_FRAMEBUFFER_WIN32_H
#define DSE_CORE_FRAMEBUFFER_WIN32_H

#include <dse/core/Image.h>
#include <dse/core/Window.h>
#include <dse/util/functional.h>
#include "win32.h"
#include <dse/core/Window_win32.h>

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
    void Render();
    void SetDrawCallback(util::function_ptr<void(void*, math::ivec2)> callback);
private:
    void OnPaint(HWND hWnd, WPARAM, LPARAM);
    void OnErase(HWND hWnd, HDC hdc, bool& result);
    void OnResize();
    void DrawDC(const swal::DC& dc, const RECT& rc);

    core::Window& window;
    std::atomic_flag sync;
    util::function_ptr<void (void*, math::ivec2)> renderCallback = nullptr;
    util::function_ptr<void ()> exitCallback = nullptr;
    Image frameBuffer{window.Size()};
    using howner = core::Window::handle_owner;
    howner idErase = window.SubscribeEvent<WindowEvent::System + WM_ERASEBKGND>({*this, util::fn_tag<&FrameBuffer_win32::OnErase>});
    howner idPaint = window.SubscribeEvent<WindowEvent::System + WM_PAINT>({*this, util::fn_tag<&FrameBuffer_win32::OnPaint>});
    howner idResize = window.SubscribeEvent<WindowEvent::Resize>({*this, util::fn_tag<&FrameBuffer_win32::OnResize>});
};

} // namespace dse::core

#endif // DSE_CORE_FRAMEBUFFER_WIN32_H
