#include "SystemLoop_win32.h"
#include <swal/hinstance.h>

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

} // namespace dse::core
