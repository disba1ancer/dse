#include <dse/core/SystemLoop.h>
#include "SystemLoop_win32.h"

namespace dse::core {

SystemLoop::SystemLoop() :
    impl()
{}

SystemLoop::SystemLoop(SystemLoop &&oth) = default;
SystemLoop::~SystemLoop() = default;
auto SystemLoop::operator=(SystemLoop &&oth) -> SystemLoop& = default;

int SystemLoop::Run()
{
    return impl->Run();
}

bool SystemLoop::RunOne()
{
    return impl->RunOne();
}

bool SystemLoop::Poll()
{
    return impl->Poll();
}

bool SystemLoop::PollOne()
{
    return impl->PollOne();
}

int SystemLoop::Result()
{
    return impl->Result();
}

void SystemLoop::Stop(int result)
{
    impl->Stop(result);
}

void SystemLoop::Post(util::function_ptr<void ()> cb)
{
    impl->Post(cb);
}

} // namespace dse::core
