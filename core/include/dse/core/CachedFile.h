#ifndef DSE_CORE_CACHEDFILE_H
#define DSE_CORE_CACHEDFILE_H

#include "File.h"

namespace dse::core {

class CachedFile
{
public:
    using Callback = File::Callback;
    using FilePos = File::FilePos;
    using FileOff = File::FileOff;

    CachedFile()
    {}

    CachedFile(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
        file(ctx, filepath, mode)
    {}

    ~CachedFile()
    {
        if (iEnd < iCurrent) {
            Flush();
        }
    }

    auto Read(std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        auto bytesRead = ReadBuffer(buf, size);
        if (bytesRead == size) {
            return {bytesRead, Make(Code::Success)};
        }
        auto remain = size - bytesRead;
        if (remain >= CacheSize) {
            auto result = CycleRead(buf + bytesRead, remain);
            return result;
        }
        iCurrent = 0;
        auto [bRead, st] = CycleRead(buffer.get(), remain, CacheSize);
        iEnd = bRead;
        bytesRead += ReadBuffer(buf, remain);
        return { bytesRead, st };
    }

	auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        auto [bytesWritten, st] = WriteBuffer(buf, size);
        if (bytesWritten == size || IsError(st))
        {
            return { bytesWritten, st };
        }
        auto remain = size - bytesWritten;
        if (remain < CacheSize) {
            return WriteBuffer(buf + bytesWritten, remain);
        }
        return CycleWrite(buf + bytesWritten, remain);
    }

    auto Resize() -> Status {
        auto st = Seek(0, StPoint::Current);
        if (IsError(st)) {
            return st;
        }
        return file.Resize();
    }

    bool ReadReady(std::size_t size)
    {
        return size <= iEnd - iCurrent;
    }

    auto CycleRead(std::byte buf[], std::size_t size, std::size_t maxSize)
        -> impl::FileOpResult
    {
        std::size_t bytesRead = 0;
        while (bytesRead < size) {
            auto [bRead, st] = file.Read(buf + bytesRead, maxSize - bytesRead);
            bytesRead += bRead;
            if (IsError(st)) {
                return {bytesRead, st};
            }
        }
        return { bytesRead, Make(Code::Success) };
    }

    auto ReadAsync(std::byte buf[], std::size_t size, const Callback& cb) -> impl::FileOpResult
    {
        auto bytesRead = ReadBuffer(buf, size);
        if (bytesRead == size) {
            cb(bytesRead, Make(Code::Success));
            return Make(Code::Success);
        }
        auto remain = size - bytesRead;
        callback = cb;
        if (remain >= CacheSize) {
            asyncBytesRead = bytesRead;
            asyncReadBuf = buf;
            auto st = file.ReadAsync(buf + bytesRead, remain, {*this, fnTag<&CachedFile::DirectReadCallback>});
            return { bytesRead, st };
        }
        return { bytesRead, Make(Code::NotImplemented) };
    }

    void DirectReadCallback(std::size_t read, Status st)
    {
        asyncBytesRead += read;
        if (IsError(st) || asyncBytesRead >= asyncSizeRead) {
            callback(asyncBytesRead, st);
        }
        st = file.ReadAsync(buf + asyncBytesRead, remain, {*this, fnTag<&CachedFile::DirectReadCallback>});
        if (st == Code::PendingOperation) {
            return;
        }
    }

    bool WriteReady(std::size_t size)
    {
        return size < CacheSize - iCurrent;
    }

	auto WriteAsync(const std::byte buf[], std::size_t size, const Callback& cb) -> Status
    {
        return file.WriteAsync(buf, size, cb);
    }

	auto Cancel() -> Status
    {
        return file.Cancel();
    }

	auto Seek(FilePos pos) -> Status
    {
        if (iEnd < iCurrent) {
            auto st = Flush();
            if (IsError(st)) {
                return st;
            }
        } else {
            iCurrent = iEnd = 0;
        }
        return file.Seek(pos);
    }

	auto Seek(FileOff offset, StPoint rel) -> Status
    {
        if (iEnd < iCurrent) {
            auto st = Flush();
            if (IsError(st)) {
                return st;
            }
        } else {
            if (rel == StPoint::Current) {
                offset += iCurrent - iEnd;
            }
            iCurrent = iEnd = 0;
        }
        return file.Seek(offset, rel);
    }

    auto OpenStatus() const -> Status
    {
        return file.OpenStatus();
    }

	auto Tell() const -> FilePos
    {
        return file.Tell() + iCurrent - iEnd;
    }

    auto Flush() -> Status
    {
        auto [bytesWritten, st] = CycleWrite(buffer.get() + iEnd, iCurrent - iEnd);
        if (IsError(st)) {
            iEnd += bytesWritten;
        } else {
            iEnd = iCurrent = 0;
        }
        return st;
    }
private:
    auto ReadBuffer(std::byte buf[], std::size_t size) -> std::size_t
    {
        auto remain = iEnd - iCurrent;
        if (remain <= 0) {
            return 0;
        }
        auto bytesRead = std::min<std::size_t>(size, remain);
        std::memcpy(buf, buffer.get() + iCurrent, bytesRead);
        iCurrent += bytesRead;
        return bytesRead;
    }

    auto CycleRead(std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        return CycleRead(buf, size, size);
    }

    auto CycleRead(std::byte buf[], std::size_t size, std::size_t maxSize)
    -> impl::FileOpResult
    {
        std::size_t bytesRead = 0;
        while (bytesRead < size) {
            auto [bRead, st] = file.Read(buf + bytesRead, maxSize - bytesRead);
            bytesRead += bRead;
            if (IsError(st)) {
                return {bytesRead, st};
            }
        }
        return { bytesRead, Make(Code::Success) };
    }

    auto WriteBuffer(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        auto remain = CacheSize - iCurrent;
        auto bytesWritten = std::min<std::size_t>(size, remain);
        std::memcpy(buffer.get() + iCurrent, buf, bytesWritten);
        iCurrent += bytesWritten;
        if (iCurrent == CacheSize) {
            return { bytesWritten, Flush() };
        }
        return { bytesWritten, Make(Code::Success) };
    }

    auto CycleWrite(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        std::size_t bytesWritten = 0;
        while (true) {
            auto [bWritten, st] = file.Write(buf + bytesWritten, size);
            bytesWritten += bWritten;
            size -= bWritten;
            if (size == 0 || IsError(st))
            {
                return { bytesWritten, st };
            }
        }
    }

    void CycleReadAsync(std::size_t bRead, Status st)
    {
        auto& buf = cycleReadAsyncBuf;
        auto& size = cycleReadAsyncSize;
        auto& maxSize = cycleReadAsyncMaxSize;
        auto& bytesRead = cycleReadAsyncBytesRead;
        switch (cycleReadStep) {
        case 0:
            bytesRead = 0;
            while (bytesRead < size) {
                cycleReadStep = 1;
                st = file.ReadAsync(buf + bytesRead, maxSize - bytesRead, {this, fnTag<&CachedFile::CycleReadAsync>});
                if (IsError(st)) {
                    cycleReadStep = -1;
                    if (size == maxSize) {
                        auto cb = std::move(callback);
                        cb(bytesRead, st);
                    } else {
                        iCurrent = 0;
                        iEnd = bytesRead;
                    }
                }
                return;
        case 1:
                bytesRead += bRead;
                if (IsError(st)) {
                    return {bytesRead, st};
                }
            }
            return { bytesRead, Make(Code::Success) };
        case -1:
        }
    }

    File file;
    static constexpr auto CacheSize = 8192;
    std::unique_ptr<std::byte[]> buffer = std::make_unique<std::byte[]>(CacheSize);
    using IndexType = std::remove_cv_t<decltype(CacheSize)>;
    IndexType iCurrent = 0;
    IndexType iEnd = 0;
    Callback callback;

    int cycleReadStep;
    std::byte* cycleReadAsyncBuf;
    std::size_t cycleReadAsyncSize;
    std::size_t cycleReadAsyncMaxSize;
    std::size_t cycleReadAsyncBytesRead;
    std::size_t asyncBytesRead;
    std::size_t asyncSizeRead;
    std::byte* asyncReadBuf;
};

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
