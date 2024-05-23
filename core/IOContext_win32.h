#ifndef DSE_CORE_IOCONTEXT_WIN32_H
#define DSE_CORE_IOCONTEXT_WIN32_H

#include <dse/core/IOContext.h>
#include <swal/handle.h>

namespace dse::core {

using CompleteCallback = void(*)(OVERLAPPED* ovl, DWORD transfered, DWORD error);

class IOContext_win32 : public std::enable_shared_from_this<IOContext_win32>
{
public:
    IOContext_win32();
    void Run();
    void RunOne();
    void Poll();
    void PollOne();
    void StopOne();
    void IOCPAttach(swal::Handle& handle, CompleteCallback cb);
    void Lock();
    bool Unlock();
    static auto GetImplFromObj(IOContext& context) -> std::shared_ptr<IOContext_win32>;
private:
    enum PollResult {
        Enqueue,
        Timeout,
        StopSig
    };
    void Post(CompleteCallback cb, OVERLAPPED* ovl, DWORD transfered);
    auto PollOne(DWORD timeout) -> PollResult;

    swal::IOCompletionPort iocp;
    std::atomic_int activeOps = 0;
};

} // namespace dse::core

#endif // DSE_CORE_IOCONTEXT_WIN32_H
