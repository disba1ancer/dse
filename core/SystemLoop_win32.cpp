#include "SystemLoop_win32.h"
#include <swal/hinstance.h>
#include <timeapi.h>

namespace dse::core {

SystemLoop_win32::SystemLoop_win32() :
    timerThreadEvent(true, false),
    timerThread(&SystemLoop_win32::TimerThreadFunc, this)
{
    msgWnd.Create(
        0, WindowClass(), TEXT("UI Thread window"), WS_POPUP,
        0, 0, 0, 0,
        HWND_MESSAGE, NULL,
        swal::GetLocalInstance(), this
    );
}

SystemLoop_win32::~SystemLoop_win32()
{
    timerThreadStop = true;
    timerThreadEvent.Set();
    while (timerThreadStop) {
        Poll();
    }
    timerThread.join();
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
    if (func == nullptr || interval == 0) {
        return {};
    }
    auto timer = AllocTimer();
    auto index = TimerIndex(timer);
    timer->handler = func;
    timer->object = obj;
    timer->interval = decltype(timer->interval){interval};
    timer->nextTime = clock::now() + timer->interval;
    scheduledTimers.emplace(timer->nextTime, index);
    timerThreadEvent.Set();
    // swal::winapi_call(::SetTimer(msgWnd, index + 1, interval / 1000, nullptr));
    return to_handle(index);
}

void SystemLoop_win32::StopPeriodic(SystemLoopTimerHandle handle) noexcept
{
    auto index = to_index(handle);
    if (index < 0 || index >= timerStore.size()) {
        return;
    }
    auto timer = TimerByIndex(index);
    scheduledTimers.erase(timer->nextTime);
    if (timer->handler == nullptr) {
        return;
    }
    // swal::winapi_call(::KillTimer(msgWnd, index + 1));
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
    case TimerMsg: {
        auto now = clock::now();
        auto end = scheduledTimers.end();
        auto cur = scheduledTimers.begin();
        if (timerThreadStop) {
            timerThreadStop = false;
            timerThreadState = TimerThreadStop;
            return 0;
        }
        if (cur == end) {
            timerThreadState = TimerThreadPaused;
            return 0;
        }
        timerThreadState = TimerThreadWaitNext;
        while (cur->first <= now) {
            while (cur->first <= now) {
                auto &timer = timerStore[cur->second];
                timer.nextTime += ((now - timer.nextTime) / timer.interval + 1) * timer.interval;
                auto id = cur->second;
                scheduledTimers.erase(cur);
                scheduledTimers.emplace(timer.nextTime, id);
                timer.handler(timer.object);
                cur = scheduledTimers.begin();
            }
            now = clock::now();
            cur = scheduledTimers.begin();
        }
        return cur->first.time_since_epoch().count();
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

auto SystemLoop_win32::TimerByIndex(ptrdiff_t index) -> timer*
{
    return timerStore.data() + index;
}

auto SystemLoop_win32::AllocTimer() -> timer*
{
    if (freeTimerHead < 0) {
        timerStore.push_back({});
        return TimerByIndex(timerStore.size() - 1);
    }
    auto timer = TimerByIndex(freeTimerHead);
    freeTimerHead += timer->nextFree;
    return timer;
}

auto SystemLoop_win32::TimerIndex(timer* timer) -> std::ptrdiff_t
{
    return timer - timerStore.data();
}

void SystemLoop_win32::FreeTimer(timer *timer) noexcept
{
    auto index = TimerIndex(timer);
    timer->handler = nullptr;
    timer->nextFree = freeTimerHead - index;
    freeTimerHead = index;
}

void SystemLoop_win32::TimerThreadFunc()
{
    ::timeBeginPeriod(1);
    DWORD timeout = INFINITE;
    while (timerThreadState) {
        timerThreadEvent.WaitFor(timeout);
        time_point nextTime{clock::duration{::SendMessage(msgWnd, TimerMsg, 0, 0)}};
        if (timerThreadState == TimerThreadWaitNext) {
            timeout = std::max(0ll, std::chrono::duration_cast<std::chrono::milliseconds>(nextTime - clock::now()).count());
        } else {
            timeout = INFINITE;
        }
        timerThreadEvent.Reset();
    }
    ::timeEndPeriod(1);
}

} // namespace dse::core
