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

void IOContext::RunOne()
{
    GetImpl()->RunOne();
}

void IOContext::Poll()
{
    GetImpl()->Poll();
}

void IOContext::PollOne()
{
    GetImpl()->PollOne();
}

void IOContext::StopOne()
{
    GetImpl()->StopOne();
}

} // namespace dse::core
