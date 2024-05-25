#include <dse/core/UILoop.h>
#include "UILoop_win32.h"

namespace dse::core {

UILoop::UILoop() :
    pimpl(std::make_shared<UILoop_impl>())
{}

int UILoop::Run()
{
    return GetImpl()->Run();
}

bool UILoop::RunOne()
{
    return GetImpl()->RunOne();
}

bool UILoop::Poll()
{
    return GetImpl()->Poll();
}

bool UILoop::PollOne()
{
    return GetImpl()->PollOne();
}

int UILoop::Result()
{
    return GetImpl()->Result();
}

void UILoop::Stop(int result)
{
    GetImpl()->Stop(result);
}

void UILoop::Post(util::FunctionPtr<void ()> cb)
{
    GetImpl()->Post(cb);
}

} // namespace dse::core
