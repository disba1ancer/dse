#ifndef DSE_CORE_CACHEDFILE_H
#define DSE_CORE_CACHEDFILE_H

#include "File.h"

namespace dse::core {

class CachedFile {
public:
    using Callback = File::Callback;
    using FilePos = File::FilePos;
    using FileOff = File::FileOff;

    CachedFile() {}

    CachedFile(IOContext &ctx, std::u8string_view filepath, OpenMode mode)
        : file(ctx, filepath, mode)
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
            return {bytesRead, Make(status::Code::Success)};
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
        return {bytesRead, st};
    }

    auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        auto [bytesWritten, st] = WriteBuffer(buf, size);
        if (bytesWritten == size || IsError(st)) {
            return {bytesWritten, st};
        }
        auto remain = size - bytesWritten;
        if (remain < CacheSize) {
            return WriteBuffer(buf + bytesWritten, remain);
        }
        return CycleWrite(buf + bytesWritten, remain);
    }

    auto Resize() -> Status
    {
        auto st = Seek(0, StPoint::Current);
        if (IsError(st)) {
            return st;
        }
        return file.Resize();
    }

    bool ReadReady(std::size_t size) { return size <= iEnd - iCurrent; }

    auto ReadAsync(std::byte buf[], std::size_t size, const Callback &cb)
        -> impl::FileOpResult
    {
        auto bytesRead = ReadBuffer(buf, size);
        if (bytesRead == size) {
            cb(bytesRead, Make(status::Code::Success));
            return {0, Make(status::Code::Success)};
        }
        iCurrent = 0;
        syncTransfered = bytesRead;
        auto remain = size - bytesRead;
        callback = cb;
        asyncTransfered = 0;
        asyncSize = remain;
        if (remain >= CacheSize) {
            asyncBuf = buf;
            return DoDirectRead(0, Make(status::Code::Success));
        }
        asyncBuf = buffer.get();
        return DoBufferedRead(0, Make(status::Code::Success));
    }

    auto DoDirectRead(std::size_t bytesRead, Status st) -> impl::FileOpResult
    {
        do {
            asyncTransfered += bytesRead;
            if (IsError(st) || asyncTransfered >= asyncSize) {
                return {asyncTransfered + syncTransfered, st};
            }
            auto remain = asyncSize - asyncTransfered;
            auto r = file.ReadAsync(
                asyncBuf + asyncTransfered, remain,
                {*this, util::fnTag<&CachedFile::DirectReadCallback>}
            );
            bytesRead = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, Make(status::Code::PendingOperation)};
    }

    void DirectReadCallback(std::size_t bytesRead, Status st)
    {
        auto r = DoDirectRead(bytesRead, st);
        if (st != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
    }

    auto DoBufferedRead(std::size_t bytesRead, Status st) -> impl::FileOpResult
    {
        do {
            asyncTransfered += bytesRead;
            if (IsError(st) || asyncTransfered >= asyncSize) {
                iEnd = asyncTransfered;
                auto r = ReadBuffer(asyncBuf + syncTransfered, asyncSize);
                return {r + syncTransfered, st};
            }
            auto remain = CacheSize - asyncTransfered;
            auto r = file.ReadAsync(
                asyncBuf + asyncTransfered, remain,
                {*this, util::fnTag<&CachedFile::BufferedReadCallback>}
                );
            bytesRead = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, Make(status::Code::PendingOperation)};
    }

    void BufferedReadCallback(std::size_t bytesRead, Status st)
    {
        auto r = DoBufferedRead(bytesRead, st);
        if (st != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
    }

    bool WriteReady(std::size_t size) { return size < CacheSize - iCurrent; }

    auto WriteAsync(const std::byte buf[], std::size_t size, const Callback &cb)
        -> impl::FileOpResult
    {
        return file.WriteAsync(buf, size, cb);
    }

    auto Cancel() -> Status { return file.Cancel(); }

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

    auto OpenStatus() const -> Status { return file.OpenStatus(); }

    auto Tell() const -> FilePos { return file.Tell() + iCurrent - iEnd; }

    auto Flush() -> Status
    {
        auto [bytesWritten, st]
            = CycleWrite(buffer.get() + iEnd, iCurrent - iEnd);
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
        return {bytesRead, Make(status::Code::Success)};
    }

    auto WriteBuffer(const std::byte buf[], std::size_t size)
        -> impl::FileOpResult
    {
        auto remain = CacheSize - iCurrent;
        auto bytesWritten = std::min<std::size_t>(size, remain);
        std::memcpy(buffer.get() + iCurrent, buf, bytesWritten);
        iCurrent += bytesWritten;
        if (iCurrent == CacheSize) {
            return {bytesWritten, Flush()};
        }
        return {bytesWritten, Make(status::Code::Success)};
    }

    auto CycleWrite(const std::byte buf[], std::size_t size)
        -> impl::FileOpResult
    {
        std::size_t bytesWritten = 0;
        while (true) {
            auto [bWritten, st] = file.Write(buf + bytesWritten, size);
            bytesWritten += bWritten;
            size -= bWritten;
            if (size == 0 || IsError(st)) {
                return {bytesWritten, st};
            }
        }
    }

    File file;
    static constexpr auto CacheSize = 4096;
    std::unique_ptr<std::byte[]> buffer =
        std::make_unique<std::byte[]>(CacheSize);
    using IndexType = std::remove_cv_t<decltype(CacheSize)>;
    IndexType iCurrent = 0;
    IndexType iEnd = 0;
    Callback callback;

    std::size_t syncTransfered;
    std::size_t asyncTransfered;
    std::size_t asyncSize;
    std::byte *asyncBuf;
};

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
