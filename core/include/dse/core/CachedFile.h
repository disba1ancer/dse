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
    CachedFile() :
        iBegin(0),
        iCurrent(0),
        iEnd(0)
    {}
    CachedFile(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
        file(ctx, filepath, mode),
        iBegin(0),
        iCurrent(0),
        iEnd(0)
    {}
    auto Read(std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        auto bytesRead = ReadBuffer(buf, size);
        if (bytesRead == size) {
            return {bytesRead, Make(status::Code::Success)};
        }
        auto remain = size - bytesRead;
        auto a = std::size(buffer) - iEnd;
        if (remain > a) {
            auto result = CycleRead(buf + bytesRead, size - bytesRead);
            iBegin = iCurrent = iEnd = (result.transfered - a) % CacheSize;
            return result;
        }
        auto [bRead, st] = CycleRead(buffer + iCurrent, remain, a);
        iEnd = iCurrent + bRead;
        iCurrent += std::min(remain, bRead);
        bytesRead = std::min(remain, bRead);
        return {bytesRead, st};
    }
	auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        return file.Write(buf, size);
    }
    auto Resize() -> Status {
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
        return file.Seek(pos);
    }
	auto Seek(FileOff offset, StPoint rel) -> Status
    {
        return file.Seek(offset, rel);
    }
    auto GetStatus() const -> Status
    {
        return file.GetStatus();
    }
	auto Tell() const -> FilePos
    {
        return file.Tell();
    }
private:
    auto ReadBuffer(std::byte buf[], std::size_t size) -> std::size_t
    {
        auto remain = iEnd - iCurrent;
        if (remain <= 0) {
            return 0;
        }
        auto bytesRead = std::min<std::size_t>(size, remain);
        std::memcpy(buf, buffer + iCurrent, bytesRead);
        iCurrent += bytesRead;
        return bytesRead;
    }
    auto CycleRead(std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        return CycleRead(buf, size, size);
    }
    auto CycleRead(std::byte buf[], std::size_t size, std::size_t maxSize) -> impl::FileOpResult
    {
        std::size_t bytesRead = 0;
        while (bytesRead < size) {
            auto [bRead, st] = file.Read(buf + bytesRead, maxSize - bytesRead);
            bytesRead += bRead;
            if (st != status::Code::Success) {
                return {bytesRead, st};
            }
        }
        return {bytesRead, Make(status::Code::Success)};
    }
    File file;
    static constexpr auto CacheSize = 8192;
    std::byte buffer[CacheSize];
    using IndexType = std::remove_cv_t<decltype(CacheSize)>;
    IndexType iBegin;
    IndexType iCurrent;
    IndexType iEnd;
    Callback callback;
};

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
