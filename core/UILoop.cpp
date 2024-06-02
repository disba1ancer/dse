#include <dse/core/UILoop.h>
#include "UILoop_win32.h"

namespace dse::core {

UILoop::UILoop() :
    impl()
{}

UILoop::UILoop(UILoop &&oth) = default;
UILoop::~UILoop() = default;
auto UILoop::operator=(UILoop &&oth) -> UILoop& = default;

int UILoop::Run()
{
    return impl->Run();
}

bool UILoop::RunOne()
{
    return impl->RunOne();
}

bool UILoop::Poll()
{
    return impl->Poll();
}

bool UILoop::PollOne()
{
    return impl->PollOne();
}

int UILoop::Result()
{
    return impl->Result();
}

void UILoop::Stop(int result)
{
    impl->Stop(result);
}

void UILoop::Post(util::FunctionPtr<void ()> cb)
{
    impl->Post(cb);
}

} // namespace dse::core
