#include "SystemLoop_win32.h"
#include <swal/hinstance.h>
#include <timeapi.h>

namespace dse::core {

SystemLoop_win32::SystemLoop_win32()
{
    msgWnd.Create(
        0, WindowClass(), TEXT("UI Thread window"), WS_POPUP,
        0, 0, 0, 0,
        HWND_MESSAGE, NULL,
        swal::GetLocalInstance(), this
    );
}

int SystemLoop_win32::Run()
{
    while(Poll()) {
        swal::winapi_call(::WaitMessage());
    }
    return Result();
}

bool SystemLoop_win32::RunOne()
{
    auto r = PollOneInt();
    if (r == PollEmpty) {
        swal::winapi_call(::WaitMessage());
        r = PollOneInt();
    }
    return r != PollQuit;
}

bool SystemLoop_win32::Poll()
{
    Constants r;
    while ((r = PollOneInt()) == PollNormal) {}
    return r != PollQuit;
}

bool SystemLoop_win32::PollOne()
{
    return PollOneInt() != PollQuit;
}

int SystemLoop_win32::Result()
{
    return msg.wParam;
}

void SystemLoop_win32::Stop(int result)
{
    PostQuitMessage(result);
}

void SystemLoop_win32::Post(util::function_ptr<void ()> cb)
{
    auto wParam = reinterpret_cast<WPARAM>(reinterpret_cast<void*>(cb.get_function()));
    auto lParam = reinterpret_cast<LPARAM>(cb.get_object_ptr());
    swal::winapi_call(PostMessage(msgWnd, PostMsg, wParam, lParam));
}

int SystemLoop_win32::Send(util::function_ptr<int ()> cb)
{
    auto wParam = reinterpret_cast<WPARAM>(reinterpret_cast<void*>(cb.get_function()));
    auto lParam = reinterpret_cast<LPARAM>(cb.get_object_ptr());
    return SendMessage(msgWnd, SendMsg, wParam, lParam);
}

namespace {

auto to_handle(std::ptrdiff_t index)
{
    return SystemLoopTimerHandle{index + 1};
}

auto to_index(SystemLoopTimerHandle handle)
{
    return std::to_underlying(handle) - 1;
}

}

auto SystemLoop_win32::Periodic(long long interval, void* obj, void(*func)(void*)) -> SystemLoopTimerHandle
{
    auto timer = AllocTimer();
    auto index = TimerIndex(timer);
    timer->handler = func;
    timer->object = obj;
    swal::winapi_call(::SetTimer(msgWnd, index + 1, interval / 1000, nullptr));
    return to_handle(index);
}

void SystemLoop_win32::StopPeriodic(SystemLoopTimerHandle handle) noexcept
{
    auto index = to_index(handle);
    if (index < 0 || index >= timerStore.size()) {
        return;
    }
    auto timer = TimerByIndex(index);
    if (timer->handler == nullptr) {
        return;
    }
    swal::winapi_call(::KillTimer(msgWnd, index + 1));
    FreeTimer(timer);
}

HWND SystemLoop_win32::OwnerWindow()
{
    return msgWnd;
}

auto SystemLoop_win32::GetImpl(SystemLoop &pub) -> SystemLoop_win32*
{
    return pub.impl;
}

auto SystemLoop_win32::WindowClass() -> LPCTSTR
{
	static swal::auto_window_class<
		SystemLoop_win32,
		&SystemLoop_win32::msgWnd,
		&SystemLoop_win32::WndProc
	> cls;
	return cls;
}

LRESULT SystemLoop_win32::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message) {
    case PostMsg: {
        auto func = reinterpret_cast<void(*)(void*)>(reinterpret_cast<void*>(wParam));
        auto obj = reinterpret_cast<void*>(lParam);
        func(obj);
        break;
    }
    case SendMsg: {
        auto func = reinterpret_cast<int(*)(void*)>(reinterpret_cast<void*>(wParam));
        auto obj = reinterpret_cast<void*>(lParam);
        return func(obj);
    }
    case WM_TIMER: {
        auto& timer = timerStore[wParam - 1];
        timer.handler(timer.object);
        break;
    }
    default:
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

auto SystemLoop_win32::PollOneInt() -> Constants
{
    if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return PollQuit;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        return PollNormal;
    }
    return PollEmpty;
}

auto SystemLoop_win32::TimerByIndex(ptrdiff_t index) -> timer_handler*
{
    return timerStore.data() + index;
}

auto SystemLoop_win32::AllocTimer() -> timer_handler*
{
    if (freeTimerHandlerHead < 0) {
        timerStore.push_back({});
        return TimerByIndex(timerStore.size() - 1);
    }
    auto timer = TimerByIndex(freeTimerHandlerHead);
    freeTimerHandlerHead += timer->next;
    return timer;
}

auto SystemLoop_win32::TimerIndex(timer_handler* timer) -> std::ptrdiff_t
{
    return timer - timerStore.data();
}

void SystemLoop_win32::FreeTimer(timer_handler *timer) noexcept
{
    auto index = TimerIndex(timer);
    timer->handler = nullptr;
    timer->next = freeTimerHandlerHead - index;
    freeTimerHandlerHead = index;
}

} // namespace dse::core
