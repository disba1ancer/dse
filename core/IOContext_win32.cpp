#include "IOContext_win32.h"

namespace dse::core {

namespace iocontext_detail {

thread_local swal::Event thrEvent{true, true};

}

IOContext_win32::IOContext_win32()
{}

void IOContext_win32::Run()
{
    while (PollOne(INFINITE) != StopSig) {}
}

void IOContext_win32::RunOne()
{
    PollOne(INFINITE);
}

void IOContext_win32::Poll()
{
    while (PollOne(0) == Enqueue) {}
}

void IOContext_win32::PollOne()
{
    PollOne(0);
}

namespace {

void StopFunc(OVERLAPPED*, DWORD, DWORD)
{}

}

void IOContext_win32::StopOne()
{
    Post(nullptr, nullptr, 0);
}

void IOContext_win32::IOCPAttach(swal::Handle &handle, CompleteCallback cb)
{
    auto vcb = reinterpret_cast<void*>(cb);
    iocp.AssocFile(handle, reinterpret_cast<ULONG_PTR>(vcb));
}

void IOContext_win32::Lock()
{
    activeOps.fetch_add(1, std::memory_order_release);
}

bool IOContext_win32::Unlock()
{
    return activeOps.fetch_sub(1, std::memory_order_acquire) == 1;
}

auto IOContext_win32::GetImplFromObj(IOContext& context) -> std::shared_ptr<IOContext_win32>
{
    return context.GetImpl();
}

void IOContext_win32::Post(CompleteCallback cb, OVERLAPPED *ovl, DWORD transfered)
{
    auto ucb = reinterpret_cast<ULONG_PTR>(reinterpret_cast<void*>(cb));
    Lock();
    iocp.PostQueuedCompletionStatus(transfered, ucb, ovl);
}

auto IOContext_win32::PollOne(DWORD timeout) -> PollResult
{
    auto enqIO = iocp.GetQueuedCompletionStatus2(0);
    if (enqIO.error != ERROR_SUCCESS) {
        if (enqIO.error == WAIT_TIMEOUT) {
            return Timeout;
        }
        if (enqIO.ovl == nullptr) {
            throw std::runtime_error("Unexpected IOCP error");
        }
    }
    auto vcb = reinterpret_cast<void*>(enqIO.key);
    auto cb = reinterpret_cast<CompleteCallback>(vcb);
    if (vcb == nullptr) {
        Unlock();
        return StopSig;
    }
    cb(enqIO.ovl, enqIO.bytesTransfered, enqIO.error);
    if (Unlock()) {
        Post(StopFunc, nullptr, 0);
        return StopSig;
    }
    return Enqueue;
}

} // namespace dse::core
