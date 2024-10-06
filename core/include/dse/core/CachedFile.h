#ifndef DSE_CORE_CACHEDFILE_H
#define DSE_CORE_CACHEDFILE_H

#include <cstring>
#include "File.h"

namespace dse::core {

class CachedFile {
public:
    using Callback = File::Callback;
    using FilePos = File::FilePos;
    using FileOff = File::FileOff;

    CachedFile() {}

    CachedFile(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
        file(ctx, filepath, mode)
    {}

    CachedFile(CachedFile&&) = default;
    CachedFile(const CachedFile&) = delete;
    CachedFile& operator=(CachedFile&&) = default;
    CachedFile& operator=(const CachedFile&) = delete;

    ~CachedFile()
    {
        if (iEnd < iCurrent) {
            Flush();
        }
    }

    auto Read(void* buf_, std::size_t size) -> raw_file_impl::FileOpResult
    {
        auto buf = static_cast<std::byte*>(buf_);
        auto bytesRead = ReadBuffer(buf, size);
        if (bytesRead == size) {
            return {bytesRead, Make(status::Code::Success)};
        }
        auto remain = size - bytesRead;
        if (remain >= CacheSize) {
            auto result = CycleRead(buf + bytesRead, remain);
            result.transferred += bytesRead;
            return result;
        }
        iCurrent = 0;
        auto [bRead, st] = CycleRead(buffer.get(), remain, CacheSize);
        iEnd = bRead;
        return {bytesRead + ReadBuffer(buf, remain), st};
    }

    auto Write(const void* buf_, std::size_t size) -> raw_file_impl::FileOpResult
    {
        auto buf = static_cast<const std::byte*>(buf_);
        auto bytesWritten = WriteBuffer(buf, size);
        if (bytesWritten == size) {
            return {bytesWritten, Make(status::Code::Success)};
        }
        auto st = Flush();
        if (status::IsError(st)) {
            return {bytesWritten, st};
        }
        auto remain = size - bytesWritten;
        if (remain < CacheSize) {
            bytesWritten += WriteBuffer(buf + bytesWritten, remain);
            return {bytesWritten, st};
        }
        auto r = CycleWrite(buf + bytesWritten, remain);
        r.transferred += bytesWritten;
        return r;
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

    auto ReadAsync(void* buf_, std::size_t size, const Callback& cb)
        -> raw_file_impl::FileOpResult
    {
        auto buf = static_cast<std::byte*>(buf_);
        auto bytesRead = ReadBuffer(buf, size);
        if (bytesRead == size) {
            return {bytesRead, Make(status::Code::Success)};
        }
        iCurrent = 0;
        iEnd = 0;
        callback = cb;
        asyncTransferred = bytesRead;
        asyncSize = size - bytesRead;
        asyncBuf = buf;
        if (asyncSize >= CacheSize) {
            return DoDirectRead(0, Make(status::Code::Success));
        }
        return DoBufferedRead(0, Make(status::Code::Success));
    }

    auto ReadAsync(void* buf_, std::size_t size)
        -> raw_file_impl::file_sender3<
            &CachedFile::ReadAsync,
            [](decltype(this) self, void* buf, std::size_t size) {
                return self->ReadReady(size);
            },
            decltype(this), decltype(buf_), decltype(size)>
    {
        return {
            std::forward<decltype(this)>(this),
            std::forward<decltype(buf_)>(buf_),
            std::forward<decltype(size)>(size)
        };
    }

    bool WriteReady(std::size_t size) { return size < CacheSize - iCurrent; }

    auto WriteAsync(const void* buf_, std::size_t size, const Callback& cb)
        -> raw_file_impl::FileOpResult
    {
        auto buf = static_cast<const std::byte*>(buf_);
        auto bytesWritten = WriteBuffer(buf, size);
        if (bytesWritten == size) {
            return {bytesWritten, Make(status::Code::Success)};
        }
        callback = cb;
        asyncTransferred = bytesWritten;
        asyncSize = size - bytesWritten;
        asyncBuf = const_cast<std::byte*>(buf);
        return DoFlushBuffer<
            &CachedFile::WriteEnding>(0, Make(status::Code::Success));
    }

    auto WriteAsync(const void* buf_, std::size_t size)
        -> raw_file_impl::file_sender3<
            &CachedFile::WriteAsync,
            [](decltype(this) self, const void* buf, std::size_t size) {
                return self->WriteReady(size);
            },
            decltype(this), decltype(buf_), decltype(size)>
    {
        return {
            std::forward<decltype(this)>(this),
            std::forward<decltype(buf_)>(buf_),
            std::forward<decltype(size)>(size)
        };
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

    bool SeekReady(FilePos pos) { return iEnd >= iCurrent; }

    auto SeekAsync(FilePos pos, Callback cb) -> Status
    {
        if (iEnd < iCurrent) {
            callback = cb;
            asyncSeekAbs = pos;
            return DoFlushBuffer<&CachedFile::SeekAbsEnding>(
                0, Make(status::Code::Success)
            ).ecode;
        } else {
            iCurrent = iEnd = 0;
        }
        return file.Seek(pos);
    }

    auto SeekAsync(FilePos pos)
        -> raw_file_impl::file_sender3<
            &CachedFile::SeekAsync,
            &std::remove_pointer_t<decltype(this)>::SeekReady, decltype(this),
            decltype(pos)>
    {
        return {
            std::forward<decltype(this)>(this), std::forward<decltype(pos)>(pos)
        };
    }

    auto Seek(FileOff offset, StPoint rel) -> Status
    {
        if (iEnd < iCurrent) {
            auto st = Flush();
            if (IsError(st)) {
                return st;
            }
            return file.Seek(offset, rel);
        }
        if (rel != StPoint::Current) {
            iCurrent = iEnd = 0;
            return file.Seek(offset, rel);
        }
        offset += iCurrent;
        if (offset < 0 || offset >= iEnd) {
            offset -= iEnd;
            iCurrent = iEnd = 0;
            return file.Seek(offset, rel);
        } else {
            iCurrent = offset;
            return Make(status::Code::Success);
        }
    }

    bool SeekReady(FileOff offset, StPoint rel) { return iEnd >= iCurrent; }

    auto SeekAsync(FileOff offset, StPoint rel, Callback cb) -> Status
    {
        if (iEnd < iCurrent) {
            callback = cb;
            asyncSeek = offset;
            asyncSeekPoint = rel;
            return DoFlushBuffer<
                &CachedFile::SeekEnding>(0, Make(status::Code::Success))
            .ecode;
        }
        return Seek(offset, rel);
    }

    auto SeekAsync(FileOff offset, StPoint rel)
        -> raw_file_impl::file_sender3<
            [](decltype(this) self, FileOff offset, StPoint rel, Callback cb) {
                return self->SeekAsync(offset, rel, cb);
            },
            [](decltype(this) self, FileOff offset, StPoint rel) {
                return self->SeekReady(offset, rel);
            },
            decltype(this), decltype(offset), decltype(rel)>
    {
        return {
            std::forward<decltype(this)>(this),
            std::forward<decltype(offset)>(offset),
            std::forward<decltype(rel)>(rel)
        };
    }

    auto OpenStatus() const -> Status { return file.OpenStatus(); }

    auto Tell() const -> FilePos { return file.Tell() + iCurrent - iEnd; }

    auto Flush() -> Status
    {
        if (iCurrent == iEnd) {
            iEnd = iCurrent = 0;
            return Make(status::Code::Success);
        }
        auto [bytesWritten, st]
            = CycleWrite(buffer.get() + iEnd, iCurrent - iEnd);
        if (IsError(st)) {
            iEnd += bytesWritten;
        } else {
            iEnd = iCurrent = 0;
        }
        return st;
    }

    auto FlushAsync(Callback cb) -> Status
    {
        callback = cb;
        asyncTransferred = 0;
        asyncSize = 0;
        asyncBuf = nullptr;
        return DoFlushBuffer<&CachedFile::FlushEnding>(0, Make(status::Code::Success)).ecode;
    }

    auto FlushAsync() -> raw_file_impl::file_sender3<
                          &CachedFile::FlushAsync,
                          [](decltype(this) self) { return false; },
                          decltype(this)>
    {
        return {std::forward<decltype(this)>(this)};
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

    auto CycleRead(std::byte buf[], std::size_t size)
        -> raw_file_impl::FileOpResult
    {
        return CycleRead(buf, size, size);
    }

    auto CycleRead(std::byte buf[], std::size_t size, std::size_t maxSize)
        -> raw_file_impl::FileOpResult
    {
        std::size_t bytesRead = 0;
        while (bytesRead < size) {
            auto [bRead, st] = file.Read(buf + bytesRead, maxSize);
            bytesRead += bRead;
            maxSize -= bytesRead;
            if (IsError(st)) {
                return {bytesRead, st};
            }
        }
        return {bytesRead, Make(status::Code::Success)};
    }

    auto WriteBuffer(const std::byte buf[], std::size_t size) -> std::size_t
    {
        auto remain = CacheSize - iCurrent;
        if (remain < size && iEnd == iCurrent) {
            return 0;
        }
        auto bytesWritten = std::min<std::size_t>(size, remain);
        std::memcpy(buffer.get() + iCurrent, buf, bytesWritten);
        iCurrent += bytesWritten;
        return bytesWritten;
    }

    auto CycleWrite(const std::byte buf[], std::size_t size)
        -> raw_file_impl::FileOpResult
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

    auto DoDirectRead(std::size_t bytesRead, Status st)
        -> raw_file_impl::FileOpResult
    {
        do {
            auto transferred = asyncTransferred + bytesRead;
            auto remain = asyncSize - bytesRead;
            if (IsError(st) || asyncSize == 0) {
                return {transferred, st};
            }
            asyncTransferred = transferred;
            asyncSize = remain;
            auto r = file.ReadAsync(
                asyncBuf + transferred, remain,
                {*this, util::fnTag<&CachedFile::DirectReadCallback>}
            );
            bytesRead = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, st};
    }

    void DirectReadCallback(std::size_t bytesRead, Status st)
    {
        auto r = DoDirectRead(bytesRead, st);
        if (r.ecode != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
    }

    auto DoBufferedRead(std::size_t bytesRead, Status st)
        -> raw_file_impl::FileOpResult
    {
        do {
            iEnd += bytesRead;
            if (IsError(st) || iEnd >= asyncSize) {
                auto r = ReadBuffer(asyncBuf + asyncTransferred, asyncSize);
                return {r + asyncTransferred, st};
            }
            auto remain = CacheSize - iEnd;
            auto r = file.ReadAsync(
                buffer.get() + iEnd, remain,
                {*this, util::fnTag<&CachedFile::BufferedReadCallback>}
            );
            bytesRead = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, st};
    }

    void BufferedReadCallback(std::size_t bytesRead, Status st)
    {
        auto r = DoBufferedRead(bytesRead, st);
        if (r.ecode != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
    }

    template <raw_file_impl::FileOpResult(CachedFile::* ending)(Status st)>
    auto DoFlushBuffer(std::size_t bytesWritten, Status st)
        -> raw_file_impl::FileOpResult
    {
        do {
            iEnd += bytesWritten;
            if (IsError(st)) {
                return (this->*ending)(st);
            }
            if (iEnd >= iCurrent) {
                iCurrent = iEnd = 0;
                return (this->*ending)(st);
            }
            auto remain = iCurrent - iEnd;
            auto r = file.WriteAsync(
                buffer.get() + iEnd, remain,
                {*this, util::fnTag<&CachedFile::FlushBufferCallback<ending>>});
            bytesWritten = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, st};
    }

    template <raw_file_impl::FileOpResult(CachedFile::* ending)(Status st)>
    void FlushBufferCallback(std::size_t bytesWritten, Status st)
    {
        auto r = DoFlushBuffer<ending>(bytesWritten, st);
        if (r.ecode != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
    }

    auto WriteEnding(Status st) -> raw_file_impl::FileOpResult
    {
        if (IsError(st)) {
            return {asyncTransferred, st};
        }
        if (asyncSize >= CacheSize) {
            return DoDirectWrite(0, st);
        }
        auto bytesWritten = WriteBuffer(asyncBuf + asyncTransferred, asyncSize);
        return {bytesWritten + asyncTransferred, st};
    }

    auto DoDirectWrite(std::size_t bytesRead, Status st)
        -> raw_file_impl::FileOpResult
    {
        do {
            auto transferred = asyncTransferred + bytesRead;
            auto remain = asyncSize - bytesRead;
            if (IsError(st) || remain == 0) {
                return {transferred, st};
            }
            asyncTransferred = transferred;
            asyncSize = remain;
            auto r = file.WriteAsync(
                asyncBuf + transferred, remain,
                {*this, util::fnTag<&CachedFile::DirectWriteCallback>}
            );
            bytesRead = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, st};
    }

    void DirectWriteCallback(std::size_t bytesRead, Status st)
    {
        auto r = DoDirectWrite(bytesRead, st);
        if (r.ecode != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
    }

    auto FlushEnding(Status st) -> raw_file_impl::FileOpResult
    {
        return {0, st};
    }

    auto SeekAbsEnding(Status st) -> raw_file_impl::FileOpResult
    {
        if (IsError(st)) {
            return {0, st};
        }
        return {0, file.Seek(asyncSeekAbs)};
    }

    auto SeekEnding(Status st) -> raw_file_impl::FileOpResult
    {
        if (IsError(st)) {
            return {0, st};
        }
        return {0, file.Seek(asyncSeek, asyncSeekPoint)};
    }

    File file;
    static constexpr auto CacheSize = 8192;
    std::unique_ptr<std::byte[]> buffer =
        std::make_unique<std::byte[]>(CacheSize);
    using IndexType = std::remove_cv_t<decltype(CacheSize)>;
    IndexType iCurrent = 0;
    IndexType iEnd = 0;
    Callback callback;

    union {
        struct {
            std::size_t asyncTransferred;
            std::size_t asyncSize;
        };
        struct {
            FileOff asyncSeek;
            StPoint asyncSeekPoint;
        };
        FilePos asyncSeekAbs;
    };
    std::byte *asyncBuf;
};

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
