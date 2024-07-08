/*
 * IOTargetwin32.cpp
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#include "File_win32.h"
#include <filesystem>
#include <algorithm>
#include <limits>
#include <dse/util/scope_exit.h>
#include <dse/util/access.h>
#include "IOContext_win32.h"

namespace {

using namespace dse::core;
using dse::core::status::Code;

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
	if (static_cast<unsigned>(mode & OpenMode::Read) != 0) {
		result += FILE_READ_DATA;
	}
	if (static_cast<unsigned>(mode & OpenMode::Write) != 0) {
		result += FILE_WRITE_DATA;
	} else if (static_cast<unsigned>(mode & OpenMode::Append) != 0) {
		result += FILE_APPEND_DATA;
	}
	return result;
}

swal::CreateMode modeToCreateMode(OpenMode mode) {
	typedef swal::CreateMode CM;
	if ((mode & (OpenMode::Read | OpenMode::Write | OpenMode::Append)) == OpenMode::Read) mode |= OpenMode::Existing;
	static const CM modeMap[] = { CM::OpenAlways, CM::CreateAlways, CM::OpenExisting, CM::TruncateExisting };
	return modeMap[ static_cast<unsigned>(mode & (OpenMode::Clear | OpenMode::Existing)) >> 3 ];
}

swal::File open(std::u8string_view filepath, OpenMode mode) {
	typedef swal::ShareMode SM;

	auto path = convertFilePath(filepath);
	return swal::File(path, modeToAccess(mode), SM::Read | SM::Write, modeToCreateMode(mode), FILE_FLAG_OVERLAPPED);
}

constexpr std::size_t maxTransferSize = std::numeric_limits<DWORD>::max();

}

namespace dse::core {

File_win32::File_win32() : handle() {
}

File_win32::File_win32(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
    context(IOContext_impl::GetImplFromObj(ctx)),
	handle()
{
	try {
		handle = open(filepath, mode);
		lastError = swal::last_error().value();
		context->IOCPAttach(handle, Complete);
	} catch (std::system_error& err) {
		SetLastError(err);
	}
}

bool File_win32::IsValid() const
{
	return handle != INVALID_HANDLE_VALUE;
}

auto File_win32::Read(std::byte buf[], std::size_t size) -> impl::FileOpResult
{
	OVERLAPPED::hEvent = iocontext_detail::IocpDisabledEvent();
	OVERLAPPED::Offset = (DWORD)pos;
	OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
	DWORD lastTransfered = 0;
	try {
        handle.Read(buf, std::min(size, maxTransferSize), *this);
        lastError = ERROR_SUCCESS;
	} catch (std::system_error& err) {
		SetLastError(err);
	}
	if (
		lastError == ERROR_SUCCESS ||
		lastError == ERROR_IO_PENDING
	) {
		try {
			lastTransfered = handle.GetOverlappedResult(*this, true);
			lastError = ERROR_SUCCESS;
			IncPtr(lastTransfered);
		} catch (std::system_error& err) {
			SetLastError(err);
		}
	}
	eof = (lastError == ERROR_HANDLE_EOF);
	return { lastTransfered, status::FromSystem(lastError) };
}

auto File_win32::Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
{
	OVERLAPPED::hEvent = iocontext_detail::IocpDisabledEvent();
	OVERLAPPED::Offset = (DWORD)pos;
	OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
	DWORD lastTransfered = 0;
	try {
        handle.Write(buf, std::min(size, maxTransferSize), *this);
        lastError = ERROR_SUCCESS;
	} catch (std::system_error& err) {
		SetLastError(err);
	}
	if (
		lastError == ERROR_SUCCESS ||
		lastError == ERROR_IO_PENDING
	) {
		try {
			lastTransfered = handle.GetOverlappedResult(*this, true);
			lastError = ERROR_SUCCESS;
			IncPtr(lastTransfered);
		} catch (std::system_error& err) {
			SetLastError(err);
		}
	}
	return { lastTransfered, status::FromSystem(lastError) };
}

void File_win32::Complete(DWORD transfered, DWORD error)
{
	File::Callback cb;
	{
		lastError = error;
		DWORD lastTransfered = transfered;
		if (error == ERROR_SUCCESS) {
			IncPtr(lastTransfered);
		} else {
			if (error == ERROR_HANDLE_EOF) {
				eof = true;
			}
		}
		cb = std::move(this->cb);
	}
	if (cb) {
		cb(transfered, status::MakeSystem(error));
	}
}

auto File_win32::Resize() -> Status
{
	LARGE_INTEGER li;
	li.LowPart = (DWORD)pos;
	li.HighPart = pos >> std::numeric_limits<DWORD>::digits;
	try {
		handle.SetPointerEx(li, swal::SetPointerModes::Begin);
		handle.SetEndOfFile();
		lastError = ERROR_SUCCESS;
	} catch (std::system_error& err) {
		SetLastError(err);
	}
	return status::FromSystem(lastError);
}

auto File_win32::ReadAsync(std::byte buf[], std::size_t size, const File::Callback& cb) -> Status
{
	OVERLAPPED::hEvent = NULL;
	OVERLAPPED::Offset = (DWORD)pos;
	OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
	this->cb = cb;
    lastError = ERROR_IO_PENDING;
    context->Lock();
	try {
        handle.Read(buf, std::min(size, maxTransferSize), *this);
        lastError = ERROR_SUCCESS;
	} catch (std::system_error& err) {
        if (SetLastError(err)) {
            return status::Make(Code::PendingOperation);
        }
	}
    context->Unlock();
	eof = lastError == ERROR_HANDLE_EOF;
    return status::FromSystem(lastError);
}

auto File_win32::WriteAsync(const std::byte buf[], std::size_t size, const File::Callback& cb) -> Status
{
	OVERLAPPED::hEvent = NULL;
	OVERLAPPED::Offset = (DWORD)pos;
	OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
	this->cb = cb;
    lastError = ERROR_IO_PENDING;
    context->Lock();
	try {
        handle.Write(buf, std::min(size, maxTransferSize), *this);
        lastError = ERROR_SUCCESS;
	} catch (std::system_error& err) {
        if (SetLastError(err)) {
            return status::Make(Code::PendingOperation);
        }
	}
    context->Unlock();
    return status::FromSystem(lastError);
}

bool File_win32::IsBusy() {
//	return event.WaitFor(0) != WAIT_OBJECT_0;
}

auto File_win32::Cancel() -> Status {
	try {
		handle.CancelIoEx();
		lastError = ERROR_SUCCESS;
	} catch (std::system_error& err) {
		SetLastError(err);
	}
	return status::FromSystem(lastError);
}

auto File_win32::GetStatus() const -> Status {
	return status::FromSystem(lastError);
}

File_win32::~File_win32() {
//	event.WaitFor(INFINITE);
}

void File_win32::Complete(OVERLAPPED *ovl, DWORD transfered, DWORD error)
{
    auto self = static_cast<File_win32*>(ovl);
    self->Complete(transfered, error);
}

void File_win32::IncPtr(DWORD transfered) {
	pos += transfered;
}

auto File_win32::Tell() const -> FilePos {
	return pos;
}

auto File_win32::Seek(FilePos pos) -> Status {
	this->pos = pos;
	eof = false;
	return {};
}

auto File_win32::Seek(FileOff off, StPoint point) -> Status {
	switch (point) {
	case StPoint::START:
		pos = std::max(FileOff(0), off);
		break;
	case StPoint::CURRENT:
		pos += FilePos(off);
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

bool File_win32::IsEOF() const {
	return eof;
}

bool File_win32::SetLastError(std::system_error &err) {
	auto ecode = err.code();
	if (ecode.category() != swal::win32_category::instance()) {
        throw;
	}
    if (ecode.value() != ERROR_IO_PENDING) {
        lastError = ecode.value();
        return false;
    }
    return true;
}

} // namespace dse::core
