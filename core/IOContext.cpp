#include <dse/core/IOContext.h>
#include "IOContext_win32.h"

namespace dse::core {

IOContext::IOContext() :
    pimpl(std::make_shared<IOContext_impl>())
{}

void IOContext::Run()
{
    GetImpl()->Run();
}

void IOContext::Stop(StopMode mode)
{
    GetImpl()->Stop(mode);
}

} // namespace dse::core
