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
    CachedFile() {}
    CachedFile(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
        file(ctx, filepath, mode)
    {}
    auto Read(std::byte buf[], std::size_t size) -> impl::FileOpResult
    {
        return file.Read(buf, size);
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
    File file;
    static constexpr auto CacheSize = 8192;
    std::byte buffer[CacheSize];
    using PtrType = std::remove_cv_t<decltype(CacheSize)>;
    PtrType current;
    PtrType end;
    Callback callback;
};

} // namespace dse::core

#endif // DSE_CORE_CACHEDFILE_H
