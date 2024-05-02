#ifndef DSE_CORE_IOCONTEXT_WIN32_H
#define DSE_CORE_IOCONTEXT_WIN32_H

#include <dse/core/IOContext.h>
#include <swal/handle.h>

namespace dse::core {

class IAsyncIO2 : public OVERLAPPED {
public:
	virtual void Complete(DWORD transfered, DWORD error) = 0;
protected:
	static std::shared_ptr<IOContext_win32> GetImplFromContext(IOContext& context);
};

class IOContext_win32 : public std::enable_shared_from_this<IOContext_win32>
{
public:
    using StopMode = IOContext::StopMode;
    using enum StopMode;
    IOContext_win32();
    void Run();
    void RunOne();
    void Poll();
    bool PollOne();
    void Stop(StopMode mode = Soft);
    void IOCPAttach(swal::Handle& handle);
private:
    swal::IOCompletionPort iocp;
};

} // namespace dse::core

#endif // DSE_CORE_IOCONTEXT_WIN32_H
