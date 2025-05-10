#ifndef DSE_CORE_UILOOP_WIN32_H
#define DSE_CORE_UILOOP_WIN32_H

#include "dse/core/SystemLoop.h"
#include "swal/handle.h"
#include <map>
#include <chrono>
#include <swal/window.h>
#include <dse/util/functional.h>
#include <thread>

namespace dse::core {

class SystemLoop_win32
{
public:
    SystemLoop_win32();
    ~SystemLoop_win32();
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
        SendMsg,
        TimerMsg,
        TimerThreadStop = 0,
        TimerThreadPaused = 1,
        TimerThreadWaitNext = 2
    };
    auto PollOneInt() -> Constants;
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    struct timer {
        void(*handler)(void*);
        union {
            std::ptrdiff_t nextFree;
            void* object;
        };
        std::chrono::microseconds interval;
        time_point nextTime;
    };
    auto TimerByIndex(std::ptrdiff_t index) -> timer*;
    auto AllocTimer() -> timer*;
    auto TimerIndex(timer* timer) -> std::ptrdiff_t;
    void FreeTimer(timer* timer) noexcept;
    void TimerThreadFunc();

    MSG msg;
    std::vector<timer> timerStore;
    std::ptrdiff_t freeTimerHead = -1;
    std::map<time_point, std::ptrdiff_t> scheduledTimers;
    bool timerThreadStop = false;
    int timerThreadState = TimerThreadPaused;
    swal::Event timerThreadEvent;
    std::thread timerThread;
    swal::Window msgWnd;
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_WIN32_H
