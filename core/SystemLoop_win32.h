#ifndef DSE_CORE_UILOOP_WIN32_H
#define DSE_CORE_UILOOP_WIN32_H

#include "dse/core/SystemLoop.h"
#include <swal/window.h>
#include <dse/util/functional.h>

namespace dse::core {

class SystemLoop_win32
{
public:
    SystemLoop_win32();
    int Run();
    bool RunOne();
    bool Poll();
    bool PollOne();
    int Result();
    void Stop(int result = 0);
    void Post(util::function_ptr<void()> cb);
    int Send(util::function_ptr<int()> cb);
    static auto GetImpl(SystemLoop& pub) -> SystemLoop_win32*;
private:
    static ATOM WindowClass();
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    enum Constants {
        GwlpThis = 0,
        PollQuit = 0,
        PollEmpty,
        PollNormal,
        PostMsg = WM_USER + 16,
        SendMsg
    };
    auto PollOneInt() -> Constants;

    swal::Window msgWnd;
    MSG msg;
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_WIN32_H
