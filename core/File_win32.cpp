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

File_win32::File_win32() :
    handle(),
    openError(ERROR_FILE_NOT_FOUND)
{}

File_win32::File_win32(IOContext& ctx, std::u8string_view filepath, OpenMode mode) :
    File_win32()
{
	try {
        handle = open(filepath, mode);
        context = IOContext_impl::GetImplFromObj(ctx);
        context->IOCPAttach(handle, Complete);
        swal::winapi_call(SetFileCompletionNotificationModes(handle, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS));
        append = bool(mode & OpenMode::Append);
    } catch (std::system_error& err) {
        auto ecode = err.code();
        if (ecode.category() != swal::win32_category::instance()) {
            throw;
        }
        openError = ecode.value();
	}
}

auto File_win32::Read(std::byte buf[], std::size_t size) -> impl::FileOpResult
{
	OVERLAPPED::hEvent = iocontext_detail::IocpDisabledEvent();
	OVERLAPPED::Offset = (DWORD)pos;
    OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
    DWORD lastTransfered = 0;
	try {
        if (!::ReadFile(handle, buf, std::min(size, maxTransferSize), &lastTransfered, this)) {
            handle.GetOverlappedResult(*this, lastTransfered, true);
        }
        IncPtr(lastTransfered);
        return {lastTransfered, Make(Code::Success)};
    } catch (std::system_error& err) {
        auto st = SysErrToStatus(err);
        IncPtr(lastTransfered);
        return {lastTransfered, st};
    }
}

auto File_win32::Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult
{
    if (append) {
        pos = -1;
    }
	OVERLAPPED::hEvent = iocontext_detail::IocpDisabledEvent();
	OVERLAPPED::Offset = (DWORD)pos;
    OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
    DWORD lastTransfered = 0;
	try {
        if (!::WriteFile(handle, buf, std::min(size, maxTransferSize), &lastTransfered, this)) {
            handle.GetOverlappedResult(*this, lastTransfered, true);
        }
        IncPtr(lastTransfered);
        return {lastTransfered, Make(Code::Success)};
    } catch (std::system_error& err) {
        auto st = SysErrToStatus(err);
        IncPtr(lastTransfered);
        return {lastTransfered, st};
    }
}

void File_win32::Complete(DWORD transfered, DWORD error)
{
    File::Callback cb;
    cb = std::move(this->cb);
    IncPtr(transfered);
	if (cb) {
        cb(transfered, (status::MakeSystem)(error));
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
    } catch (std::system_error& err) {
        return SysErrToStatus(err);
	}
    return Make(Code::Success);
}

auto File_win32::ReadAsync(std::byte buf[], std::size_t size, const File::Callback& cb) -> impl::FileOpResult
{
	OVERLAPPED::hEvent = NULL;
	OVERLAPPED::Offset = (DWORD)pos;
	OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
    DWORD lastTransfered = 0;
    this->cb = cb;
    context->Lock();
	try {
        if (handle.Read(buf, std::min(size, maxTransferSize), lastTransfered, *this)) {
            IncPtr(lastTransfered);
            context->Unlock();
            return { lastTransfered, Make(Code::Success) };
        }
        return { lastTransfered, Make(Code::PendingOperation) };
    } catch (std::system_error& err) {
        auto st = SysErrToStatus(err);
        ::GetOverlappedResult(handle, this, &lastTransfered, FALSE);
        IncPtr(lastTransfered);
        context->Unlock();
        return {lastTransfered, st};
    }
}

auto File_win32::WriteAsync(const std::byte buf[], std::size_t size, const File::Callback& cb) -> impl::FileOpResult
{
    if (append) {
        pos = -1;
    }
	OVERLAPPED::hEvent = NULL;
	OVERLAPPED::Offset = (DWORD)pos;
	OVERLAPPED::OffsetHigh = pos >> std::numeric_limits<DWORD>::digits;
    DWORD lastTransfered = 0;
    this->cb = cb;
    context->Lock();
	try {
        if (handle.Write(buf, std::min(size, maxTransferSize), lastTransfered, *this)) {
            IncPtr(lastTransfered);
            context->Unlock();
            return {lastTransfered, Make(Code::Success)};
        }
        return { lastTransfered, Make(Code::PendingOperation) };
    } catch (std::system_error& err) {
        auto st = SysErrToStatus(err);
        ::GetOverlappedResult(handle, this, &lastTransfered, FALSE);
        IncPtr(lastTransfered);
        context->Unlock();
        return {lastTransfered, st};
    }
}

auto File_win32::Cancel() -> Status
{
	try {
        handle.CancelIoEx();
	} catch (std::system_error& err) {
        return SysErrToStatus(err);
	}
    return Make(Code::Success);
}

auto File_win32::OpenStatus() const -> Status
{
    if (handle == INVALID_HANDLE_VALUE) {
        return status::FromSystem(openError);
    }
    return Make(Code::Success);
}

File_win32::~File_win32()
{}

void File_win32::Complete(OVERLAPPED *ovl, DWORD transfered, DWORD error)
{
    auto self = static_cast<File_win32*>(ovl);
    self->Complete(transfered, error);
}

auto File_win32::SysErrToStatus(std::system_error &err) -> Status
{
    auto ecode = err.code();
    if (ecode.category() != swal::win32_category::instance()) {
        throw;
    }
    return (status::FromSystem)(ecode.value());
}

void File_win32::IncPtr(DWORD transfered)
{
    if (pos == -1) {
        pos = 0;
    } else {
        pos += transfered;
    }
}

auto File_win32::Tell() const -> FilePos {
	return pos;
}

auto File_win32::Seek(FilePos pos) -> Status {
    this->pos = pos;
	return {};
}

auto File_win32::Seek(FileOff off, StPoint point) -> Status {
	switch (point) {
	case StPoint::Start:
		pos = std::max(FileOff(0), off);
		break;
	case StPoint::Current:
		pos += FilePos(off);
		pos = (off < 0 && pos > FilePos(off) ? FilePos(0) : pos);
		break;
    case StPoint::End: {
		LARGE_INTEGER li;
		GetFileSizeEx(handle, &li);
		FilePos eofpos = li.HighPart;
		eofpos <<= (sizeof(li.LowPart) * CHAR_BIT);
		eofpos += li.LowPart;
        off = -std::min(FileOff(0), off);
        pos = eofpos - std::min<FilePos>(off, eofpos);
        break;
    }
    }
    return Make(Code::Success);
}

} // namespace dse::core
