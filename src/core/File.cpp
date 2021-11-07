/*
 * IOTarget.cpp
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#include "File.h"

#ifdef _WIN32
#include "File_win32.h"
#endif

namespace dse::core {

File::File() : impl(new IOTarget_impl()) {
}

File::File(ThreadPool& pool, std::u8string_view filepath, OpenMode mode) : impl(new IOTarget_impl(pool, filepath, mode)) {
}

bool File::IsValid() const {
	return impl->IsValid();
}

auto File::Read(std::byte buf[], std::size_t size) -> impl::FileOpResult {
	return impl->Read(buf, size);
}

auto File::Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult {
	return impl->Write(buf, size);
}

auto File::Tell() const -> FilePos {
	return impl->Tell();
}

auto File::Seek(FilePos pos) -> std::error_code {
	return impl->Seek(pos);
}

auto File::Seek(FileOff offset, StPoint rel) -> std::error_code {
	return impl->Seek(offset, rel);
}

std::error_code File::Status() const {
	return impl->Status();
}

auto File::Resize() -> std::error_code {
	return impl->Resize();
}

bool File::IsEOF() const {
	return impl->IsEOF();
}

void File::ReadAsync(std::byte buf[], std::size_t size, const Callback& cb) {
	return impl->ReadAsync(buf, size, cb);
}

void File::WriteAsync(const std::byte buf[], std::size_t size, const Callback& cb) {
	return impl->WriteAsync(buf, size, cb);
}

auto File::Cancel() -> std::error_code {
	return impl->Cancel();
}

} // namespace dse::core
