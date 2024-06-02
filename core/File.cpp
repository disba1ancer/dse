/*
 * IOTarget.cpp
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#include <dse/core/File.h>

#ifdef _WIN32
#include "File_win32.h"
#endif

namespace dse::core {

File::File() : impl()
{}

File &File::operator=(File &&oth) = default;

File::File(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
    impl(ctx, filepath, mode)
{}

File::File(File &&oth) = default;

File::~File()
{}

bool File::IsValid() const
{
	return impl->IsValid();
}

auto File::Read(std::byte buf[], std::size_t size) -> impl::FileOpResult
{
	return impl->Read(buf, size);
}

auto File::Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
{
	return impl->Write(buf, size);
}

auto File::Tell() const -> FilePos
{
	return impl->Tell();
}

auto File::Seek(FilePos pos) -> Status
{
	return impl->Seek(pos);
}

auto File::Seek(FileOff offset, StPoint rel) -> Status
{
	return impl->Seek(offset, rel);
}

auto File::GetStatus() const -> Status
{
    return impl->GetStatus();
}

auto File::Resize() -> Status
{
	return impl->Resize();
}

bool File::IsEOF() const
{
	return impl->IsEOF();
}

auto File::ReadAsync(std::byte buf[], std::size_t size, const Callback& cb) -> Status
{
	return impl->ReadAsync(buf, size, cb);
}

auto File::WriteAsync(const std::byte buf[], std::size_t size, const Callback& cb) -> Status
{
	return impl->WriteAsync(buf, size, cb);
}

auto File::Cancel() -> Status {
	return impl->Cancel();
}

} // namespace dse::core
