/*
 * IOTargetwin32.cpp
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#include "File_win32.h"
#include <filesystem>
#include <algorithm>
#include "IOCP_win32.h"
#include "../../util/FinalStep.h"
#include "../../util/Access.h"

namespace {
std::wstring convertFilePath(std::u8string_view filepath) {
	auto path = std::filesystem::path(filepath);
	path = path.lexically_normal();
	std::wstring rslt;
	if (path.native().length() > MAX_PATH - 1) {
		path = std::filesystem::absolute(path);
		rslt.reserve(path.native().length() + 5);
		(rslt += LR"(//?/)") += path.native();
	} else {
		rslt = path.native();
	}
	return rslt;
}

DWORD modeToAccess(dse::os::io::OpenMode mode) {
	using dse::os::io::OpenMode;
	DWORD result = 0;
	if (static_cast<unsigned>(mode & OpenMode::READ) != 0) {
		result += FILE_READ_DATA;
	}
	if (static_cast<unsigned>(mode & OpenMode::WRITE) != 0) {
		result += FILE_WRITE_DATA;
	} else if (static_cast<unsigned>(mode & OpenMode::APPEND) != 0) {
		result += FILE_APPEND_DATA;
	}
	return result;
}

DWORD modeToCreateMode(dse::os::io::OpenMode mode) {
	using dse::os::io::OpenMode;
	if ((mode & (OpenMode::READ | OpenMode::WRITE | OpenMode::APPEND)) == OpenMode::READ) mode |= OpenMode::EXISTING;
	static const DWORD modeMap[] = { OPEN_ALWAYS, CREATE_ALWAYS, OPEN_EXISTING, TRUNCATE_EXISTING };
	return modeMap[ static_cast<unsigned>(mode & (OpenMode::CLEAR | OpenMode::EXISTING)) >> 3 ];
}

HANDLE open(std::u8string_view filepath, dse::os::io::OpenMode mode, DWORD& lastError) {
	auto path = convertFilePath(filepath);
	auto handle = CreateFile(path.c_str(), modeToAccess(mode), FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, modeToCreateMode(mode), FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	lastError = GetLastError();
	return handle;
}

void shiftAr(char8_t *str, size_t size) {
	for (size_t i = size; i > 1; --i) {
		str[i - 1] = str[i - 2];
	}
}

int c32tou8(char8_t *str, char32_t ch) {
	if (ch <= 127) {
		str[0] = ch;
	} else {
		int bytesCount = 0;
		while (ch) {
			shiftAr(str, 4);
			if (ch >= (1U << (6U - bytesCount))) {
				str[0] = (ch & 0x3FU) | 0x80U;
				++bytesCount;
			} else {
				++bytesCount;
				auto mask = 0xFU << (8U - bytesCount);
				str[0] = (ch & 0x3FU) | mask;
			}
			ch >>= 6;
		}
		return bytesCount;
	}
	return 1;
}

std::u8string wcTou8(std::wstring_view in) {
	enum {
		SurrogateA    = 0b1101100000000000,
		SurrogateB    = 0b1101110000000000,
		SurrogateMask = 0b1111110000000000,
		CodeMask      = 0b0000001111111111,
		Shift         = 10,
		Base          = 0x10000
	};
	std::u8string result;
	result.reserve(in.size() * sizeof(std::wstring_view::value_type));
	char8_t u8code[4];
	char32_t code;
	for (std::size_t i = 0; bool((code = in[i])) || (i < in.size()); ++i) {
		switch ((code & SurrogateMask)) {
		case SurrogateA:
			++i;
			code = ((code & CodeMask) << Shift) + (in[i] & CodeMask) + Base;
			break;
		case SurrogateB:
			continue;
		}
		result += std::u8string_view(u8code, u8code + c32tou8(u8code, code));
	}
	return result;
}

}

namespace dse {
namespace os {
namespace io {

File_win32::File_win32() : handle(INVALID_HANDLE_VALUE) {
}

File_win32::File_win32(std::u8string_view filepath, OpenMode mode) :
	handle(open(filepath, mode, lastError))
{
	error = handle == INVALID_HANDLE_VALUE;
	if (!error) {
		IOCP_win32::instance.attach(this);
	}
}

/*IOTarget_win32::IOTarget_win32(IOTarget_win32 &&other) : ovl(std::move(other.ovl)), handle(other.handle), event(std::move(other.event)), pos(other.pos) {
	other.handle = INVALID_HANDLE_VALUE;
}

IOTarget_win32& IOTarget_win32::operator =(
		IOTarget_win32 &&other) {
	std::swap(ovl, other.ovl);
	std::swap(handle, other.handle);
	std::swap(event, other.event);
	std::swap(pos, other.pos);
	return *this;
}*/

bool File_win32::isValid() const {
	return handle != INVALID_HANDLE_VALUE;
}

void File_win32::read(std::byte buf[], std::size_t size) {
	std::lock_guard lck(dataMtx);
	ovl.ovl.hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(event.operator HANDLE()) | 1);
	ovl.ovl.Offset = pos;
	ovl.ovl.OffsetHigh = pos >> (sizeof(ovl.ovl.Offset) * CHAR_BIT);
	ovl.target = this;
	error = !ReadFile(handle, buf, size, nullptr, &ovl.ovl);
	lastError = GetLastError();
	if (error && lastError != ERROR_IO_PENDING) {
		lastTransfered = 0;
	} else {
		error = !GetOverlappedResult(handle, &ovl.ovl, &lastTransfered, TRUE);
		lastError = GetLastError();
		if (!error) {
			incPtr(lastTransfered);
		} else {
			if (lastError == ERROR_HANDLE_EOF) {
				error = false;
				eof = true;
			}
		}
	}
}

void File_win32::write(std::byte buf[], std::size_t size) {
	std::lock_guard lck(dataMtx);
	ovl.ovl.hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(event.operator HANDLE()) | 1);
	ovl.ovl.Offset = pos;
	ovl.ovl.OffsetHigh = pos >> (sizeof(ovl.ovl.Offset) * CHAR_BIT);
	ovl.target = this;
	error = !WriteFile(handle, buf, size, nullptr, &ovl.ovl);
	lastError = GetLastError();
	if (error && lastError != ERROR_IO_PENDING) {
		lastTransfered = 0;
	} else {
		error = !GetOverlappedResult(handle, &ovl.ovl, &lastTransfered, TRUE);
		lastError = GetLastError();
		if (!error) {
			incPtr(lastTransfered);
		}
	}
}

void File_win32::complete(DWORD transfered, DWORD error) {
	std::function<void()> cb;
	{
		std::lock_guard lck(dataMtx);
		util::FinalStep fin([this]{ callback = nullptr; });
		lastError = error;
		lastTransfered = transfered;
		if (!(this->error = (error != ERROR_SUCCESS))) {
			incPtr(lastTransfered);
		} else {
			if (lastError == ERROR_HANDLE_EOF) {
				this->error = false;
				eof = true;
			}
		}
		cb = std::move(callback);
		if (references < 2) cb = nullptr;
	}
	if (cb) cb();
	release();
}

void File_win32::resize() {
	std::lock_guard lck(dataMtx);
	LARGE_INTEGER li;
	li.LowPart = pos;
	li.HighPart = pos >> (sizeof(li.LowPart) * CHAR_BIT);
	if (!((error = !SetFilePointerEx(handle, li, nullptr, FILE_BEGIN)) && (lastError = GetLastError()))) {
		if (!((error = !SetEndOfFile(handle)) && (lastError = GetLastError()))) {
			lastError = ERROR_SUCCESS;
		}
	}
}

void File_win32::read_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish) {
	std::lock_guard lck(dataMtx);
	ovl.ovl.hEvent = event;
	ovl.ovl.Offset = pos;
	ovl.ovl.OffsetHigh = pos >> (sizeof(ovl.ovl.Offset) * CHAR_BIT);
	ovl.target = this;
	callback = std::move(onFinish);
	auto result = ReadFile(handle, buf, size, nullptr, &ovl.ovl);
	lastError = GetLastError();
	error = (!result && lastError != ERROR_IO_PENDING);
	references += !error;
	error = (error && lastError != ERROR_HANDLE_EOF);
	eof = lastError == ERROR_HANDLE_EOF;
	lastTransfered *= (!error);
}

void File_win32::write_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish) {
	std::lock_guard lck(dataMtx);
	ovl.ovl.hEvent = event;
	ovl.ovl.Offset = pos;
	ovl.ovl.OffsetHigh = pos >> (sizeof(ovl.ovl.Offset) * CHAR_BIT);
	ovl.target = this;
	callback = std::move(onFinish);
	auto result = WriteFile(handle, buf, size, nullptr, &ovl.ovl);
	lastError = GetLastError();
	error = (!result && lastError != ERROR_IO_PENDING);
	references += !error;
	lastTransfered *= (!error);
}

bool File_win32::isBusy() {
	return WaitForSingleObject(event, 0) == WAIT_OBJECT_0;
}

void File_win32::cancel() {
	std::lock_guard lck(dataMtx);
	if (!CancelIoEx(handle, nullptr)) {
		lastError = GetLastError();
	}
}

File_win32::~File_win32() {
	IOCP_win32::instance.preventCallback(this);
	WaitForSingleObject(event, INFINITE);
}

void File_win32::release() {
	int r;
	{
		std::lock_guard lck(dataMtx);
		r = (--references);
	}
	if (r <= 0) delete this;
}

void File_win32::incPtr(DWORD transfered) {
	pos += transfered;
}

FilePos File_win32::tell() const {
	std::lock_guard lck(dataMtx);
	return pos;
}

void File_win32::seek(FilePos pos) {
	std::lock_guard lck(dataMtx);
	this->pos = pos;
	eof = false;
}

void File_win32::seek(FileOff off, StPoint point) {
	std::lock_guard lck(dataMtx);
	switch (point) {
	case StPoint::START:
		pos = std::max(FileOff(0), off);
		break;
	case StPoint::CURRENT:
		pos += off;
		pos = (off < 0 && pos > FilePos(off) ? FilePos(0) : pos);
		break;
	case StPoint::END: {
		LARGE_INTEGER li;
		GetFileSizeEx(handle, &li);
		FilePos eofpos = li.HighPart;
		eofpos <<= (sizeof(li.LowPart) * CHAR_BIT);
		eofpos += li.LowPart;
		pos = eofpos + off;
		pos = (off < 0 && pos > FilePos(off) ? FilePos(0) : pos);
	}
		break;
	}
	eof = false;
}

bool File_win32::isEOF() const {
	std::lock_guard lck(dataMtx);
	return eof;
}

bool File_win32::isError() const {
	std::lock_guard lck(dataMtx);
	return error;
}

Result File_win32::getLastResult() const {
	auto lError = util::access(dataMtx, lastError);
	switch (lError) {
	case ERROR_SUCCESS: return Result::SUCCESS;
	case ERROR_HANDLE_EOF: return Result::END_OF_FILE;
	case ERROR_IO_PENDING: return Result::ASYNC_OPERATION;
	default: return Result::UNKNOWN;
	}
}

std::size_t File_win32::getTransfered() const {
	std::lock_guard lck(dataMtx);
	return lastTransfered;
}

std::u8string File_win32::getResultString() const {
	constexpr std::size_t resultStringMaxSize = 512;
	wchar_t buf[resultStringMaxSize];
	auto lError = util::access(dataMtx, lastError);
	auto size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, lError, 0, buf, resultStringMaxSize, nullptr);
	return wcTou8(std::wstring_view(buf, buf + size));
}

void File_win32::incRefs() {
	++references;
}

} /* namespace io */
} /* namespace os */
} /* namespace dse */

namespace dse {
namespace os {
namespace io {

void IOTargetDelete::operator()(IOTarget_impl *obj) {
	obj->release();
}

HandleWrapper::HandleWrapper(HANDLE handle) : handle(handle) {
}

HandleWrapper::~HandleWrapper() {}

HandleWrapper::HandleWrapper(HandleWrapper &&orig) : handle(orig.handle) {
	orig.handle = NULL;
}

HandleWrapper& HandleWrapper::operator =(HandleWrapper &&orig) {
	std::swap(handle, orig.handle);
	return *this;
}

HandleWrapper::operator HANDLE() const {
	return handle;
}

} /* namespace io */
} /* namespace os */
} /* namespace dse */
