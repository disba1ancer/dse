#ifndef DSE_CORE_IOCONTEXT_H
#define DSE_CORE_IOCONTEXT_H

#include "detail/impexp.h"
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32
using IOContext_impl = class IOContext_win32;
#endif

class API_DSE_CORE IOContext : dse::util::pimpl<IOContext, IOContext_impl>
{
public:
    friend IOContext_impl;
    IOContext();
    void Run();
    void RunOne();
    void Poll();
    void PollOne();
    void StopOne();
};

} // namespace dse::core

#endif // DSE_CORE_IOCONTEXT_H
