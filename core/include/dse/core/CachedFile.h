#ifndef DSE_CORE_CACHEDFILE_H
#define DSE_CORE_CACHEDFILE_H

#include <cstring>
#include "File.h"

namespace dse::core {

namespace cached_file_impl {

template <typename TagOp>
struct file_sender;

struct flush_sender;

} // namespace cached_file_impl

class CachedFile {
public:
    using Callback = File::Callback;
    using FilePos = File::FilePos;
    using FileOff = File::FileOff;

    CachedFile() {}

    CachedFile(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
        file(ctx, filepath, mode)
    {}

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
        -> cached_file_impl::file_sender<raw_file_impl::TagRead>;

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
        return DoFlushBuffer(0, Make(status::Code::Success));
    }

    auto WriteAsync(const void* buf_, std::size_t size)
        -> cached_file_impl::file_sender<raw_file_impl::TagWrite>;

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
        return DoFlushBuffer(0, Make(status::Code::Success)).ecode;
    }

    auto FlushAsync() -> cached_file_impl::flush_sender;

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

    auto DoFlushBuffer(std::size_t bytesWritten, Status st)
        -> raw_file_impl::FileOpResult
    {
        do {
            iEnd += bytesWritten;
            if (IsError(st)) {
                return {asyncTransferred, st};
            }
            if (iEnd >= iCurrent) {
                iCurrent = iEnd = 0;
                if (asyncSize == 0) {
                    return {asyncTransferred, st};
                }
                if (asyncSize >= CacheSize) {
                    return DoDirectWrite(0, st);
                }
                bytesWritten = WriteBuffer(asyncBuf + asyncTransferred, asyncSize);
                return {bytesWritten + asyncTransferred, st};
            }
            auto remain = iCurrent - iEnd;
            auto r = file.WriteAsync(
                buffer.get() + iEnd, remain,
                {*this, util::fnTag<&CachedFile::FlushBufferCallback>}
                );
            bytesWritten = r.transferred;
            st = r.ecode;
        } while (st != status::Code::PendingOperation);
        return {0, st};
    }

    void FlushBufferCallback(std::size_t bytesWritten, Status st)
    {
        auto r = DoFlushBuffer(bytesWritten, st);
        if (r.ecode != status::Code::PendingOperation) {
            callback(r.transferred, r.ecode);
        }
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
            asyncTransferred = remain;
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

    File file;
    static constexpr auto CacheSize = 8192;
    std::unique_ptr<std::byte[]> buffer =
        std::make_unique<std::byte[]>(CacheSize);
    using IndexType = std::remove_cv_t<decltype(CacheSize)>;
    IndexType iCurrent = 0;
    IndexType iEnd = 0;
    Callback callback;

    std::size_t asyncTransferred;
    std::size_t asyncSize;
    std::byte *asyncBuf;
};

namespace cached_file_impl {

using raw_file_impl::FileOpBufT;
using raw_file_impl::FileOpResult;
using raw_file_impl::TagRead;
using raw_file_impl::TagWrite;

template <typename TagOp>
struct file_sender;

template <typename TagOp>
auto do_op(
    CachedFile* file, FileOpBufT<TagOp>* buf, std::size_t size, File::Callback cb
) -> FileOpResult;

template <>
inline auto do_op<TagRead>(
    CachedFile* file, void* buf, std::size_t size, File::Callback cb
) -> FileOpResult
{
    return file->ReadAsync(buf, size, cb);
}

template <>
inline auto do_op<TagWrite>(
    CachedFile* file, const void* buf, std::size_t size, File::Callback cb
) -> FileOpResult
{
    return file->WriteAsync(buf, size, cb);
}

template <typename TagOp>
bool do_test(CachedFile* file, std::size_t size);

template <>
inline bool do_test<TagRead>(CachedFile* file, std::size_t size)
{
    return file->ReadReady(size);
}

template <>
inline bool do_test<TagWrite>(CachedFile* file, std::size_t size)
{
    return file->WriteReady(size);
}

template <typename TagOp>
struct file_awaiter {
    file_awaiter(file_sender<TagOp>& sender) :
        sender(sender)
    {}
    bool await_ready()
    {
        if (!do_test<TagOp>(sender.file, sender.size)) {
            return false;
        }
        result = do_op<TagOp>(sender.file, sender.buf, sender.size, nullptr);
        return true;
    }
    void callback(std::size_t size, Status st)
    {
        result.transferred = size;
        result.ecode = st;
        resumable.resume();
    }
    bool await_suspend(std::coroutine_handle<> handle)
    {
        FileOpResult r = do_op<TagOp>(
            sender.file, sender.buf, sender.size,
            {*this, util::fnTag<&file_awaiter::callback>}
        );
        if (r.ecode != status::Code::PendingOperation) {
            result = r;
            return false;
        }
        resumable = handle;
        return true;
    }
    auto await_resume() -> FileOpResult { return result; }

private:
    file_sender<TagOp>& sender;
    FileOpResult result;
    std::coroutine_handle<> resumable;
};

template <typename TagOp>
struct file_sender {
    file_sender(CachedFile* file, FileOpBufT<TagOp>* buf, std::size_t size) :
        file(file),
        buf(buf),
        size(size)
    {}

    friend class file_awaiter<TagOp>;

    friend auto operator co_await(file_sender<TagOp>&& sndr
    ) -> file_awaiter<TagOp>
    {
        return {sndr};
    }

    friend auto operator co_await(file_sender<TagOp>& sndr
    ) -> file_awaiter<TagOp>
    {
        return {sndr};
    }

private:
    CachedFile* file;
    FileOpBufT<TagOp>* buf;
    std::size_t size;
};

struct flush_awaiter;

struct flush_sender {
    flush_sender(CachedFile* file) :
        file(file)
    {}

    friend class flush_awaiter;

private:
    CachedFile* file;
};

struct flush_awaiter {
    flush_awaiter(flush_sender&& sender) :
        sender(std::move(sender))
    {}
    bool await_ready() { return false; }
    void callback(std::size_t size, Status st)
    {
        result = st;
        resumable.resume();
    }
    bool await_suspend(std::coroutine_handle<> handle)
    {
        Status st = sender.file->FlushAsync(
            {*this, util::fnTag<&flush_awaiter::callback>}
        );
        if (st != status::Code::PendingOperation) {
            result = st;
            return false;
        }
        resumable = handle;
        return true;
    }
    auto await_resume() -> Status { return result; }

private:
    flush_sender sender;
    Status result;
    std::coroutine_handle<> resumable;
};

auto operator co_await(flush_sender&& sndr) -> flush_awaiter
{
    return {std::move(sndr)};
}

auto operator co_await(flush_sender& sndr) -> flush_awaiter
{
    return {std::move(sndr)};
}

} // namespace cached_file_impl

auto CachedFile::ReadAsync(void* buf, std::size_t size)
    -> cached_file_impl::file_sender<raw_file_impl::TagRead>
{
    return {this, buf, size};
}

auto CachedFile::WriteAsync(const void* buf, std::size_t size)
    -> cached_file_impl::file_sender<raw_file_impl::TagWrite>
{
    return {this, buf, size};
}

auto CachedFile::FlushAsync() -> cached_file_impl::flush_sender
{
    return {this};
}

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
