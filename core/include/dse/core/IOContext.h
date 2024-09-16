#ifndef DSE_CORE_IOCONTEXT_H
#define DSE_CORE_IOCONTEXT_H

#include "detail/impexp.h"
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32
using IOContext_impl = class IOContext_win32;
#endif

class API_DSE_CORE IOContext
{
public:
    friend IOContext_impl;
    IOContext();
    ~IOContext();
    int Run();
    int RunOne();
    int Poll();
    int PollOne();
    void StopOne();
    void MakePersistent(bool persist = true);
private:
    util::impl_ptr<IOContext_impl> impl;
};

} // namespace dse::core

#endif // DSE_CORE_IOCONTEXT_H
