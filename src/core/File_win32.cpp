/*
 * IOTargetwin32.cpp
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#include "File_win32.h"
#include <filesystem>
#include <algorithm>
#include "util/scope_exit.h"
#include "util/access.h"
#include "errors_win32.h"

namespace {

using namespace dse::core;

std::wstring convertFilePath(std::u8string_view filepath) {
	auto path = std::filesystem::path(filepath);
	path = path.lexically_normal();
	std::wstring rslt;
	if (path.native().length() > MAX_PATH - 1) {
		path = std::filesystem::absolute(path);
		//rslt.reserve(path.native().length() + 5);
		rslt.append(LR"(//?/)").append(path.native());
	} else {
		rslt = path.native();
	}
	return rslt;
}

DWORD modeToAccess(OpenMode mode) {
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

swal::CreateMode modeToCreateMode(OpenMode mode) {
	typedef swal::CreateMode CM;
	if ((mode & (OpenMode::READ | OpenMode::WRITE | OpenMode::APPEND)) == OpenMode::READ) mode |= OpenMode::EXISTING;
	static const CM modeMap[] = { CM::OpenAlways, CM::CreateAlways, CM::OpenExisting, CM::TruncateExisting };
	return modeMap[ static_cast<unsigned>(mode & (OpenMode::CLEAR | OpenMode::EXISTING)) >> 3 ];
}

swal::File open(std::u8string_view filepath, OpenMode mode) {
	typedef swal::ShareMode SM;

	auto path = convertFilePath(filepath);
	return swal::File(path, modeToAccess(mode), SM::Read | SM::Write, modeToCreateMode(mode), FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED);
}

}

namespace dse::core {

File_win32::File_win32() : handle() {
}

File_win32::File_win32(ThreadPool& pool, std::u8string_view filepath, OpenMode mode) :
	handle()
{
	try {
		handle = open(filepath, mode);
		lastError = GetLastError();
		getImplFromPool(pool)->iocpAttach(handle);
//		IOCP_win32::instance.attach(this);
	} catch (swal::error& err) {
		lastError = err.get();
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

auto File_win32::read(std::byte buf[], std::size_t size) -> Result {
	std::lock_guard lck(dataMtx);
	OVERLAPPED::hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(HANDLE(event)) | 1);
	OVERLAPPED::Offset = pos;
	OVERLAPPED::OffsetHigh = pos >> (sizeof(OVERLAPPED::Offset) * CHAR_BIT);
	DWORD lastTransfered = 0;
	try {
		handle.Read(buf, size, *this);
		lastError = ERROR_SUCCESS;
	} catch (swal::error& err) {
		lastError = err.get();
	}
	if (lastError == ERROR_SUCCESS || lastError == ERROR_IO_PENDING) {
		try {
			lastTransfered = handle.GetOverlappedResult(*this, true);
			lastError = ERROR_SUCCESS;
			incPtr(lastTransfered);
		} catch (swal::error& err) {
			lastError = err.get();
		}
	}
	eof = (lastError == ERROR_HANDLE_EOF);
	return { lastTransfered, std::error_code(lastError, win32_category::instance()) };
	/*error = !ReadFile(handle, buf, size, nullptr, &ovl.ovl);
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
	}*/
}

auto File_win32::write(std::byte buf[], std::size_t size) -> Result {
	std::lock_guard lck(dataMtx);
	OVERLAPPED::hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(event.operator HANDLE()) | 1);
	OVERLAPPED::Offset = pos;
	OVERLAPPED::OffsetHigh = pos >> (sizeof(OVERLAPPED::Offset) * CHAR_BIT);
	DWORD lastTransfered = 0;
	try {
		handle.Write(buf, size, *this);
		lastError = ERROR_SUCCESS;
	} catch (swal::error& err) {
		lastError = err.get();
	}
	if (lastError == ERROR_SUCCESS || lastError == ERROR_IO_PENDING) {
		try {
			lastTransfered = handle.GetOverlappedResult(*this, true);
			lastError = ERROR_SUCCESS;
			incPtr(lastTransfered);
		} catch (swal::error& err) {
			lastError = err.get();
		}
	}
	return { lastTransfered, std::error_code(lastError, win32_category::instance()) };
	/*error = !WriteFile(handle, buf, size, nullptr, &ovl.ovl);
	lastError = GetLastError();
	if (error && lastError != ERROR_IO_PENDING) {
		lastTransfered = 0;
	} else {
		error = !GetOverlappedResult(handle, &ovl.ovl, &lastTransfered, TRUE);
		lastError = GetLastError();
		if (!error) {
			incPtr(lastTransfered);
		}
	}*/
}

void File_win32::complete(DWORD transfered, DWORD error) {
	{
		std::lock_guard lck(dataMtx);
		lastError = error;
		DWORD lastTransfered = transfered;
		if (error == ERROR_SUCCESS) {
			incPtr(lastTransfered);
		} else {
			if (lastError == ERROR_HANDLE_EOF) {
				eof = true;
			}
		}
	}
	pr.set_value({ transfered, std::error_code(error, win32_category::instance()) });
	release();
}

auto File_win32::resize() -> std::error_code {
	std::lock_guard lck(dataMtx);
	LARGE_INTEGER li;
	li.LowPart = pos;
	li.HighPart = pos >> (sizeof(li.LowPart) * CHAR_BIT);
	try {
		handle.SetPointerEx(li, swal::SetPointerModes::Begin);
		handle.SetEndOfFile();
		lastError = ERROR_SUCCESS;
	} catch (swal::error& err) {
		lastError = err.get();
	}
	return { int(lastError), win32_category::instance() };
	/*if (!((error = !SetFilePointerEx(handle, li, nullptr, FILE_BEGIN)) && (lastError = GetLastError()))) {
		if (!((error = !SetEndOfFile(handle)) && (lastError = GetLastError()))) {
			lastError = ERROR_SUCCESS;
		}
	}*/
}

auto File_win32::readAsync(std::byte buf[], std::size_t size) -> util::future<Result> {
	std::lock_guard lck(dataMtx);
	OVERLAPPED::hEvent = event;
	OVERLAPPED::Offset = pos;
	OVERLAPPED::OffsetHigh = pos >> (sizeof(OVERLAPPED::Offset) * CHAR_BIT);
	pr = util::promise<Result>();
	try {
		handle.Read(buf, size, *this);
		lastError = ERROR_SUCCESS;
	} catch (swal::error& err) {
		lastError = err.get();
		if (lastError == ERROR_IO_PENDING) {
			++references;
		}
	}
	eof = lastError == ERROR_HANDLE_EOF;
	return pr.get_future();
	/*auto result = ReadFile(handle, buf, size, nullptr, &ovl.ovl);
	lastError = GetLastError();
	error = (!result && lastError != ERROR_IO_PENDING);
	references += !error;
	error = (error && lastError != ERROR_HANDLE_EOF);
	eof = lastError == ERROR_HANDLE_EOF;
	lastTransfered *= (!error);*/
}

auto File_win32::writeAsync(std::byte buf[], std::size_t size) -> util::future<Result> {
	std::lock_guard lck(dataMtx);
	OVERLAPPED::hEvent = event;
	OVERLAPPED::Offset = pos;
	OVERLAPPED::OffsetHigh = pos >> (sizeof(OVERLAPPED::Offset) * CHAR_BIT);
	pr = util::promise<Result>();
	try {
		handle.Write(buf, size, *this);
		lastError = ERROR_SUCCESS;
	} catch (swal::error& err) {
		lastError = err.get();
		if (lastError == ERROR_IO_PENDING) {
			++references;
		}
	}
	return pr.get_future();
//	auto result = WriteFile(handle, buf, size, nullptr, this);
//	lastError = GetLastError();
//	error = (!result && lastError != ERROR_IO_PENDING);
//	references += !error;
//	lastTransfered *= (!error);
}

bool File_win32::isBusy() {
	return event.WaitFor(0) != WAIT_OBJECT_0;
}

auto File_win32::cancel() -> std::error_code {
	std::lock_guard lck(dataMtx);
	try {
		handle.CancelIoEx();
		lastError = ERROR_SUCCESS;
	} catch (swal::error& err) {
		lastError = err.get();
	}
	return { int(lastError), win32_category::instance() };
}

auto File_win32::status() const -> std::error_code {
	return { int(lastError), win32_category::instance() };
}

File_win32::~File_win32() {
	event.WaitFor(INFINITE);
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

auto File_win32::tell() const -> FilePos {
	std::lock_guard lck(dataMtx);
	return pos;
}

auto File_win32::seek(FilePos pos) -> std::error_code {
	std::lock_guard lck(dataMtx);
	this->pos = pos;
	eof = false;
	return {};
}

auto File_win32::seek(FileOff off, StPoint point) -> std::error_code {
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
	return {};
}

bool File_win32::isEOF() const {
	std::lock_guard lck(dataMtx);
	return eof;
}

void File_win32::incRefs() {
	++references;
}

} // namespace dse::core

namespace dse::core {

void IOTargetDelete::operator()(IOTarget_impl *obj) {
	obj->release();
}

/*HandleWrapper::HandleWrapper(HANDLE handle) : handle(handle) {
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
}*/

} // namespace dse::core
