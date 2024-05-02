#include "IOContext_win32.h"

namespace dse::core {

IOContext_win32::IOContext_win32()
{}

void IOContext_win32::Run()
{
    while (true) {
        RunOne();
    }
}

void IOContext_win32::RunOne()
{
    auto ioCompletion = iocp.GetQueuedCompletionStatus2(INFINITE);
    if (ioCompletion.ovl == nullptr) {
        return;
    }
    auto asyncHandle = static_cast<IAsyncIO2*>(ioCompletion.ovl);
    asyncHandle->Complete(ioCompletion.bytesTransfered, ioCompletion.error);
}

void IOContext_win32::Poll()
{
    while (PollOne()) {}
}

bool IOContext_win32::PollOne()
{
    auto ioCompletion = iocp.GetQueuedCompletionStatus2(0);
    if (ioCompletion.error == WAIT_TIMEOUT) {
        return false;
    }
    if (ioCompletion.ovl == nullptr) {
        return true;
    }
    auto asyncHandle = static_cast<IAsyncIO2*>(ioCompletion.ovl);
    asyncHandle->Complete(ioCompletion.bytesTransfered, ioCompletion.error);
    return true;
}

void IOContext_win32::Stop(StopMode mode)
{

}

void IOContext_win32::IOCPAttach(swal::Handle &handle)
{
    iocp.AssocFile(handle, 0x0DB5);
}

auto IAsyncIO2::GetImplFromContext(IOContext &context) -> std::shared_ptr<IOContext_win32>
{
    return context.GetImpl();
}

} // namespace dse::core
