#ifndef DSE_CORE_IOCONTEXT_H
#define DSE_CORE_IOCONTEXT_H

#include "detail/impexp.h"
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32
typedef class IOContext_win32 IOContext_impl;
#endif

class IAsyncIO2;

class API_DSE_CORE IOContext : dse::util::pimpl<IOContext, IOContext_impl>
{
public:
    friend IAsyncIO2;
    enum StopMode {
        Soft,
        Hard
    };
    IOContext();
    void Run();
    void Stop(StopMode mode = Soft);
};

} // namespace dse::core

#endif // DSE_CORE_IOCONTEXT_H
