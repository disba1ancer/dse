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

bool File::isValid() const {
	return impl->isValid();
}

auto File::read(std::byte buf[], std::size_t size) -> Result {
	return impl->read(buf, size);
}

auto File::write(std::byte buf[], std::size_t size) -> Result {
	return impl->write(buf, size);
}

File::File(File &&other) = default;
File& File::operator=(File &&other) = default;
File::~File() noexcept = default;

auto File::tell() const -> FilePos {
	return impl->tell();
}

auto File::seek(FilePos pos) -> std::error_code {
	return impl->seek(pos);
}

auto File::seek(FileOff offset, StPoint rel) -> std::error_code {
	return impl->seek(offset, rel);
}

std::error_code File::status() const {
	return impl->status();
}

auto File::resize() -> std::error_code {
	return impl->resize();
}

bool File::isEOF() const {
	return impl->isEOF();
}

auto File::readAsync(std::byte buf[], std::size_t size) -> util::future<Result> {
	return impl->readAsync(buf, size);
}

auto File::writeAsync(std::byte buf[], std::size_t size) -> util::future<Result> {
	return impl->writeAsync(buf, size);
}

auto File::cancel() -> std::error_code {
	return impl->cancel();
}

} // namespace dse::core
