#ifndef DSE_CORE_UILOOP_WIN32_H
#define DSE_CORE_UILOOP_WIN32_H

#include <swal/window.h>
#include <dse/util/functional.h>

namespace dse::core {

class UILoop_win32
{
public:
    UILoop_win32();
    int Run();
    bool RunOne();
    bool Poll();
    bool PollOne();
    int Result();
    void Stop(int result = 0);
    void Post(util::FunctionPtr<void()> cb);
private:
    static ATOM WindowClass();
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    enum Constants {
        GwlpThis = 0,
        PollQuit = 0,
        PollEmpty,
        PollNormal,
        PostMsg = WM_USER + 10
    };
    auto PollOneInt() -> Constants;

    swal::Window msgWnd;
    MSG msg;
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_WIN32_H
