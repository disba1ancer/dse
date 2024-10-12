#ifndef DSE_CORE_UILOOP_H
#define DSE_CORE_UILOOP_H

#include "detail/impexp.h"
#include <dse/util/functional.h>
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32
using SystemLoop_impl = class SystemLoop_win32;
#endif

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
private:
    dse::util::impl_ptr<SystemLoop_impl> impl;
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_H
