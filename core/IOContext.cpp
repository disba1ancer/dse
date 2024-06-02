#include <dse/core/IOContext.h>
#include "IOContext_win32.h"

namespace dse::core {

IOContext::IOContext() :
    impl()
{}

IOContext::~IOContext()
{}

void IOContext::Run()
{
    impl->Run();
}

void IOContext::RunOne()
{
    impl->RunOne();
}

void IOContext::Poll()
{
    impl->Poll();
}

void IOContext::PollOne()
{
    impl->PollOne();
}

void IOContext::StopOne()
{
    impl->StopOne();
}

} // namespace dse::core
