#ifndef DSE_CORE_UILOOP_H
#define DSE_CORE_UILOOP_H

#include "detail/impexp.h"
#include <dse/util/functional.h>
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32
using UILoop_impl = class UILoop_win32;
#endif

class API_DSE_CORE UILoop
{
    friend UILoop_impl;
public:
    UILoop();
    UILoop(UILoop&& oth);
    ~UILoop();
    UILoop& operator=(UILoop&& oth);
    int Run();
    bool RunOne();
    bool Poll();
    bool PollOne();
    int Result();
    void Stop(int result);
    void Post(util::FunctionPtr<void()> cb);
private:
    dse::util::impl_ptr<UILoop_impl> impl;
};

} // namespace dse::core

#endif // DSE_CORE_UILOOP_H
