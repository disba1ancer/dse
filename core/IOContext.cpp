#include <dse/core/IOContext.h>
#include "IOContext_win32.h"

namespace dse::core {

IOContext::IOContext() :
    impl()
{}

IOContext::~IOContext()
{}

int IOContext::Run()
{
    return impl->Run();
}

int IOContext::RunOne()
{
    return impl->RunOne();
}

int IOContext::Poll()
{
    return impl->Poll();
}

int IOContext::PollOne()
{
    return impl->PollOne();
}

void IOContext::StopOne()
{
    impl->StopOne();
}

void IOContext::MakePersistent(bool persist)
{
    impl->MakePersistent(persist);
}

} // namespace dse::core
