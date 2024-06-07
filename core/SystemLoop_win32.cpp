#include "SystemLoop_win32.h"

namespace dse::core {

SystemLoop_win32::SystemLoop_win32() :
    msgWnd(WindowClass(), NULL, this)
{}

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

void SystemLoop_win32::Post(util::FunctionPtr<void ()> cb)
{
    auto wParam = reinterpret_cast<WPARAM>(reinterpret_cast<void*>(cb.GetFunction()));
    auto lParam = reinterpret_cast<LPARAM>(cb.GetObjectPtr());
    swal::winapi_call(PostMessage(msgWnd, PostMsg, wParam, lParam));
}

int SystemLoop_win32::Send(util::FunctionPtr<int ()> cb)
{
    auto wParam = reinterpret_cast<WPARAM>(reinterpret_cast<void*>(cb.GetFunction()));
    auto lParam = reinterpret_cast<LPARAM>(cb.GetObjectPtr());
    return SendMessage(msgWnd, SendMsg, wParam, lParam);
}

auto SystemLoop_win32::GetImpl(SystemLoop &pub) -> SystemLoop_win32*
{
    return pub.impl;
}

ATOM SystemLoop_win32::WindowClass()
{
    static ATOM clsID = []{
        HINSTANCE hInst = GetModuleHandle(nullptr);
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
                wcex.lpfnWndProc = swal::ClsWndProc<SystemLoop_win32, &SystemLoop_win32::WndProc, GwlpThis>;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = hInst;
		wcex.hIcon = NULL;
		wcex.hCursor = NULL;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = TEXT("dse.core.UILoop");
		wcex.hIconSm = NULL;

		return swal::winapi_call(RegisterClassEx(&wcex));
	}();
    return clsID;
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
