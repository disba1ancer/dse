#include "FrameBuffer_win32.h"
#include <dse/core/ImageManipulator.h>
#include <dse/core/WindowData_win32.h>
#include <dse/core/WindowEventData_win32.h>
#include <dse/util/scope_exit.h>
#include <dse/core/ThreadPool.h>

namespace dse::core {

void FrameBuffer_win32_Deleter::operator()(unsigned char*ptr) const
{
    ::operator delete(ptr);
}

FrameBuffer_win32::FrameBuffer_win32(core::Window& wnd) :
    window(wnd),
    paintCon(window.SubscribePaintEvent(util::FunctionPtr{*this, util::fnTag<&FrameBuffer_win32::OnPaint>})),
    resizeCon(window.SubscribeResizeEvent(util::FunctionPtr{*this, util::fnTag<&FrameBuffer_win32::OnResize>}))
{}

FrameBuffer_win32::~FrameBuffer_win32()
{}

void FrameBuffer_win32::Render(util::FunctionPtr<void ()> callback)
{
    while (sync.test_and_set(std::memory_order_relaxed));
    std::atomic_thread_fence(std::memory_order_acquire);
    exitCallback = (callback ? callback : util::fnTag<+[]{}>);
    sync.clear(std::memory_order_release);
    swal::Wnd(window.GetSysData().hWnd).InvalidateRect(true);
}

void FrameBuffer_win32::SetDrawCallback(util::FunctionPtr<void (void *, math::ivec2)> callback)
{
    renderCallback = callback;
}

void FrameBuffer_win32::OnPaint(WndEvtDt data)
{
    swal::Wnd wnd(data.hWnd);
    auto acqrd = !sync.test_and_set(std::memory_order_relaxed);
    util::scope_exit final([&wnd, &acqrd, this]{
        if (acqrd) {
            auto callback = std::exchange(exitCallback, nullptr);
            sync.clear(std::memory_order_release);
            if (callback) {
                auto pool = core::ThreadPool::GetCurrentPool();
                pool.Schedule(callback);
            }
        }
    });
    swal::PaintDC dc(wnd);
    if (!acqrd) {
        return;
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    auto size = frameBuffer.Size();
    if (size.x() == 0 || size.y() == 0) {
        return;
    }
    if (exitCallback && renderCallback) {
        renderCallback(frameBuffer.Data(), size);
    }
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

    auto xBeg = std::clamp(int(dc->rcPaint.left), 0, size.x());
    auto yBeg = std::clamp(int(dc->rcPaint.top), 0, size.y());
    auto xSiz = std::clamp(int(dc->rcPaint.right), 0, size.x()) - xBeg;
    auto ySiz = std::clamp(int(dc->rcPaint.bottom), 0, size.y()) - yBeg;
    swal::winapi_call(::SetDIBitsToDevice(dc, xBeg, yBeg, xSiz, ySiz, xBeg, 0, yBeg, ySiz, frameBuffer.Data(), &bmi, 0));
}

void FrameBuffer_win32::OnResize(WndEvtDt, int w, int h, WindowShowCommand)
{
    math::ivec2 size = {w, h};
    frameBuffer = {size};
    ImageManipulator manip(frameBuffer);
    manip.Fill({0, 0}, size, 0x00000000);
}

} // namespace dse::core
