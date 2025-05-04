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
    int  Run();
    bool RunOne();
    bool Poll();
    bool PollOne();
    int  Result();
    void Stop(int result = 0);
    void Post(util::function_ptr<void()> cb);
    int  Send(util::function_ptr<int()> cb);
    auto Periodic(long long interval, void* obj, void(*func)(void*)) -> SystemLoopTimerHandle;
    void StopPeriodic(SystemLoopTimerHandle handle) noexcept;
    HWND OwnerWindow();
    static auto GetImpl(SystemLoop& pub) -> SystemLoop_win32*;
private:
    static auto WindowClass() -> LPCTSTR;
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
    struct timer_handler {
        void(*handler)(void*);
        union {
            std::ptrdiff_t next;
            void* object;
        };
    };
    auto TimerByIndex(std::ptrdiff_t index) -> timer_handler*;
    auto AllocTimer() -> timer_handler*;
    auto TimerIndex(timer_handler* timer) -> std::ptrdiff_t;
    void FreeTimer(timer_handler* timer) noexcept;

    MSG msg;
    std::vector<timer_handler> timerStore;
    std::ptrdiff_t freeTimerHandlerHead = -1;
    swal::Window msgWnd;
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_WIN32_H
