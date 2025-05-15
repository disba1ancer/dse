#include "FrameBuffer_win32.h"
#include <dse/core/ImageManipulator.h>
#include <dse/util/scope_exit.h>
#include <dse/core/ThreadPool.h>

namespace dse::core {

FrameBuffer_win32::FrameBuffer_win32(core::Window& wnd) :
    window(wnd)
{}

FrameBuffer_win32::~FrameBuffer_win32()
{}

void FrameBuffer_win32::Render()
{
    // while (sync.test_and_set(std::memory_order_relaxed));
    // std::atomic_thread_fence(std::memory_order_acquire);
    // exitCallback = (callback ? callback : util::fn_tag<+[]{}>);
    // sync.clear(std::memory_order_release);
    auto size = frameBuffer.Size();
    // if (renderCallback) {
    //     renderCallback(frameBuffer.Data(), size);
    // }
    auto wnd = swal::Wnd(window.GetSysData().hWnd);
    swal::winapi_call(::RedrawWindow(wnd, nullptr, NULL, RDW_INVALIDATE /*| RDW_UPDATENOW*/));
    // auto dc = swal::Wnd(window.GetSysData().hWnd).GetDC();
    // if (size.x() == 0 || size.y() == 0) {
    //     return;
    // }
    // DrawDC(dc, {0, 0, size.x(), size.y()});
}

void FrameBuffer_win32::SetDrawCallback(util::function_ptr<void (void *, math::ivec2)> callback)
{
    renderCallback = callback;
}

void FrameBuffer_win32::OnPaint(HWND hWnd, WPARAM, LPARAM)
{
    swal::Wnd wnd(hWnd);
    // auto acqrd = !sync.test_and_set(std::memory_order_relaxed);
    // util::scope_exit final([&wnd, &acqrd, this]{
    //     if (acqrd) {
    //         auto callback = std::exchange(exitCallback, nullptr);
    //         sync.clear(std::memory_order_release);
    //         if (callback) {
    //             callback();
    //         }
    //     }
    // });
    RECT rc;
    if (!::GetUpdateRect(wnd, &rc, FALSE)) {
        return;
    }
    swal::WindowDC dc(wnd);
    util::scope_exit e{[&wnd]{
        wnd.ValidateRect();
    }};
    // RECT const& rc = dc->rcPaint;
    // if (!acqrd) {
    //     return;
    // }
    // std::atomic_thread_fence(std::memory_order_acquire);
    auto size = frameBuffer.Size();
    if (size.x() == 0 || size.y() == 0) {
        return;
    }
    if (renderCallback) {
        renderCallback(frameBuffer.Data(), size);
    }
    DrawDC(dc, rc);
}

void FrameBuffer_win32::OnErase(HWND hWnd, HDC hdc, bool& result)
{
    result = true;
}

void FrameBuffer_win32::OnResize()
{
    math::ivec2 size = window.SurfaceSize();
    frameBuffer = {size};
    // if (renderCallback) {
    //     renderCallback(frameBuffer.Data(), size);
    // }
}

void FrameBuffer_win32::DrawDC(const swal::DC& dc, const RECT& rc)
{
    auto size = frameBuffer.Size();
    ::BITMAPINFO bmi;
    auto &bmih = bmi.bmiHeader;
    bmih.biSize = sizeof(bmih);
    bmih.biWidth = size.x();
    bmih.biHeight = -size.y();
    bmih.biPlanes = 1;
    bmih.biBitCount = 32;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    auto xBeg = std::clamp(int(rc.left), 0, size.x());
    auto yBeg = std::clamp(int(rc.top), 0, size.y());
    auto xSiz = std::clamp(int(rc.right), 0, size.x()) - xBeg;
    auto ySiz = std::clamp(int(rc.bottom), 0, size.y()) - yBeg;
    swal::winapi_call(::SetDIBitsToDevice(
        dc,
        xBeg, yBeg, xSiz, ySiz,
        xBeg, size.y() - (yBeg + ySiz),
        0, size.y(),
        frameBuffer.Data(), &bmi, DIB_RGB_COLORS));
}

} // namespace dse::core
