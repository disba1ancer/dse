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
        Flush();
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
        return { bytesRead, st };
    }

	auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        auto [bytesWritten, st] = WriteBuffer(buf, size);
        if (bytesWritten == size ||
            st != Make(status::Code::Success))
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
        auto st = Flush();
        if (st != Make(status::Code::Success)) {
            return st;
        }
        return file.Resize();
    }

    auto ReadAsync(std::byte buf[], std::size_t size, const Callback& cb) -> Status
    {
        return file.ReadAsync(buf, size, cb);
    }

	auto WriteAsync(const std::byte buf[], std::size_t size, const Callback& cb) -> Status
    {
        return file.WriteAsync(buf, size, cb);
    }

	auto Cancel() -> Status
    {
        return file.Cancel();
    }

	bool IsEOF() const
    {
        return file.IsEOF();
    }

	bool IsValid() const
    {
        return file.IsValid();
    }

	auto Seek(FilePos pos) -> Status
    {
        if (iEnd < iCurrent) {
            auto st = Flush();
            if (st != Make(status::Code::Success)) {
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
            if (st != Make(status::Code::Success)) {
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

    auto GetStatus() const -> Status
    {
        return file.GetStatus();
    }

	auto Tell() const -> FilePos
    {
        return file.Tell() + iCurrent - iEnd;
    }

    auto Flush() -> Status
    {
        auto [s, err] = file.Write(buffer.get(), iCurrent);
        iCurrent = iEnd = 0;
        return err;
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
            if (st != status::Code::Success) {
                return {bytesRead, st};
            }
        }
        return { bytesRead, Make(status::Code::Success) };
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
        return { bytesWritten, Make(status::Code::Success) };
    }

    auto CycleWrite(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        std::size_t bytesWritten = 0;
        while (true) {
            auto [bWritten, st] = file.Write(buf + bytesWritten, size);
            bytesWritten += bWritten;
            size -= bWritten;
            if (size == 0 ||
                st != Make(status::Code::Success))
            {
                continue;
            }
            return { bytesWritten, st };
        }
    }

    File file;
    static constexpr auto CacheSize = 8192;
    std::unique_ptr<std::byte[]> buffer = std::make_unique<std::byte[]>(CacheSize);
    using IndexType = std::remove_cv_t<decltype(CacheSize)>;
    IndexType iCurrent = 0;
    IndexType iEnd = 0;
//    bool bufferDirty = false;
    Callback callback;
};

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
