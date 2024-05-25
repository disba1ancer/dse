#ifndef DSE_CORE_UILOOP_H
#define DSE_CORE_UILOOP_H

#include "detail/impexp.h"
#include <dse/util/functional.h>
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32
using UILoop_impl = class UILoop_win32;
#endif

class API_DSE_CORE UILoop : dse::util::pimpl<UILoop, UILoop_impl>
{
public:
    UILoop();
    int Run();
    bool RunOne();
    bool Poll();
    bool PollOne();
    int Result();
    void Stop(int result);
    void Post(util::FunctionPtr<void()> cb);
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_H
