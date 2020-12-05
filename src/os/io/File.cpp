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

namespace dse {
namespace os {
namespace io {

File::File() : impl(new IOTarget_impl()) {
}

File::File(threadutils::ThreadPool& pool, std::u8string_view filepath, OpenMode mode) : impl(new IOTarget_impl(pool, filepath, mode)) {
}

bool File::isValid() const {
	return impl->isValid();
}

File& File::read(std::byte buf[], std::size_t size) {
	impl->read(buf, size);
	return *this;
}

File& File::write(std::byte buf[], std::size_t size) {
	impl->write(buf, size);
	return *this;
}

File::File(File &&other) = default;
File& File::operator=(File &&other) = default;
File::~File() noexcept = default;

FilePos File::tell() const {
	return impl->tell();
}

File& File::seek(FilePos pos) {
	impl->seek(pos);
	return *this;
}

File& File::seek(FileOff offset, StPoint rel) {
	impl->seek(offset, rel);
	return *this;
}

File& File::resize() {
	impl->resize();
	return *this;
}

bool File::isEOF() const {
	return impl->isEOF();
}

bool File::isError() const {
	return impl->isError();
}

Result File::getLastResult() const {
	return impl->getLastResult();
}

std::size_t File::getTransfered() const {
	return impl->getTransfered();
}

std::u8string File::getResultString() const {
	return impl->getResultString();
}

File& File::read_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish) {
	impl->read_async(buf, size, std::move(onFinish));
	return *this;
}

File& File::write_async(std::byte buf[], std::size_t size, std::function<void()> &&onFinish) {
	impl->write_async(buf, size, std::move(onFinish));
	return *this;
}

bool File::isBusy() {
	return impl->isBusy();
}

File& File::cancel() {
	impl->cancel();
	return *this;
}

} /* namespace io */
} /* namespace os */
} /* namespace dse */
