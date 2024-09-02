#ifndef DSE_CORE_IOCONTEXT_WIN32_H
#define DSE_CORE_IOCONTEXT_WIN32_H

#include <atomic>
#include <dse/core/IOContext.h>
#include <swal/handle.h>

namespace dse::core {

namespace iocontext_detail {

extern thread_local swal::Event thrEvent;

inline auto IocpDisabledEvent() -> HANDLE
{
    auto eventLong = reinterpret_cast<ULONG_PTR>(HANDLE(thrEvent));
	return reinterpret_cast<HANDLE>(eventLong | 1);
}

}

using CompleteCallback = void(*)(OVERLAPPED* ovl, DWORD transfered, DWORD error);

class IOContext_win32
{
public:
    IOContext_win32();
    int Run();
    int RunOne();
    int Poll();
    int PollOne();
    void StopOne();
    void IOCPAttach(swal::Handle& handle, CompleteCallback cb);
    int Lock();
    int Unlock();
    static auto GetImplFromObj(IOContext& context) -> IOContext_win32*;
private:
    enum PollResult {
        Dequeue,
        Timeout,
        StopSig
    };
    void Post(CompleteCallback cb, OVERLAPPED* ovl, DWORD transfered);
    auto PollOne(int& queryCount, DWORD timeout) -> PollResult;

    swal::IOCompletionPort iocp;
    std::atomic_int activeOps = 0;
};

} // namespace dse::core

#endif // DSE_CORE_IOCONTEXT_WIN32_H
