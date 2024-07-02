#include "IOContext_win32.h"

namespace dse::core {

namespace iocontext_detail {

thread_local swal::Event thrEvent{true, true};

}

IOContext_win32::IOContext_win32()
{}

int IOContext_win32::Run()
{
    int queryCount;
    while (PollOne(queryCount, INFINITE) != StopSig) {}
    return queryCount;
}

int IOContext_win32::RunOne()
{
    int queryCount;
    PollOne(queryCount, INFINITE);
    return queryCount;
}

int IOContext_win32::Poll()
{
    int queryCount;
    while (PollOne(queryCount, 0) == Dequeue) {}
    return queryCount;
}

int IOContext_win32::PollOne()
{
    int queryCount;
    PollOne(queryCount, 0);
    return queryCount;
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

int IOContext_win32::Lock()
{
    return activeOps.fetch_add(1, std::memory_order_release) + 1;
}

int IOContext_win32::Unlock()
{
    return activeOps.fetch_sub(1, std::memory_order_acquire) - 1;
}

auto IOContext_win32::GetImplFromObj(IOContext& context) -> IOContext_win32*
{
    return context.impl;
}

void IOContext_win32::Post(CompleteCallback cb, OVERLAPPED *ovl, DWORD transfered)
{
    auto ucb = reinterpret_cast<ULONG_PTR>(reinterpret_cast<void*>(cb));
    Lock();
    iocp.PostQueuedCompletionStatus(transfered, ucb, ovl);
}

auto IOContext_win32::PollOne(int& queryCount, DWORD timeout) -> PollResult
{
    auto enqIO = iocp.GetQueuedCompletionStatus2(0);
    if (enqIO.error != ERROR_SUCCESS) {
        if (enqIO.error == WAIT_TIMEOUT) {
            queryCount = activeOps.load(std::memory_order_relaxed);
            return Timeout;
        }
        if (enqIO.ovl == nullptr) {
            throw std::runtime_error("Unexpected IOCP error");
        }
    }
    auto vcb = reinterpret_cast<void*>(enqIO.key);
    auto cb = reinterpret_cast<CompleteCallback>(vcb);
    if (vcb == nullptr) {
        queryCount = Unlock();
        return StopSig;
    }
    cb(enqIO.ovl, enqIO.bytesTransfered, enqIO.error);
    if ((queryCount = Unlock()) == 0) {
        Post(StopFunc, nullptr, 0);
        return StopSig;
    }
    return Dequeue;
}

} // namespace dse::core
