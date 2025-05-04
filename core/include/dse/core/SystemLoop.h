#ifndef DSE_CORE_UILOOP_H
#define DSE_CORE_UILOOP_H

#include "detail/impexp.h"
#include <dse/util/functional.h>
#include <dse/util/pimpl.h>
#include <dse/util/handle.h>

namespace dse::core {

#ifdef _WIN32
using SystemLoop_impl = class SystemLoop_win32;
#endif

enum class SystemLoopTimerHandle : std::ptrdiff_t {};

class API_DSE_CORE SystemLoop;

struct SystemLoopTimerHandleTrait
{
    using sender = SystemLoop;
    static void kill_handle(sender& sysLoop, SystemLoopTimerHandle handle);
};

} // namespace dse::core

template <>
struct dse::util::handle_traits<dse::core::SystemLoopTimerHandle> :
    dse::core::SystemLoopTimerHandleTrait
{};

namespace dse::core {

class API_DSE_CORE SystemLoop
{
    friend SystemLoop_impl;
public:
    SystemLoop();
    SystemLoop(SystemLoop&& oth);
    ~SystemLoop();
    SystemLoop& operator=(SystemLoop&& oth);
    int Run();
    bool RunOne();
    bool Poll();
    bool PollOne();
    int Result();
    void Stop(int result);
    void Post(util::function_ptr<void()> cb);
    auto Periodic(long long interval, void* obj, void(*func)(void*)) -> SystemLoopTimerHandle;
    auto Periodic(long long interval, util::function_ptr<void()> cb) -> dse::util::handle_owner<SystemLoopTimerHandle>;
    void StopPeriodic(SystemLoopTimerHandle handle);
private:
    dse::util::impl_ptr<SystemLoop_impl> impl;
};

inline void SystemLoopTimerHandleTrait::kill_handle(sender& sysLoop, SystemLoopTimerHandle handle)
{
    sysLoop.StopPeriodic(handle);
}

inline auto SystemLoop::Periodic(long long interval, util::function_ptr<void()> cb)
    -> dse::util::handle_owner<SystemLoopTimerHandle>
{
    return {*this, Periodic(interval, cb.get_object_ptr(), cb.get_function())};
}

} // namespace dse::core

#endif // DSE_CORE_UILOOP_H
