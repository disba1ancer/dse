/*
 * handle.h
 *
 *  Created on: 6 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_HANDLE_H_
#define WIN32_HANDLE_H_

#include <windows.h>
#include <string>
#include "enum_bitwise.h"
#include "error.h"
#include "strconv.h"
#include "zero_or_resource.h"

namespace swal {

class Handle : public zero_or_resource<HANDLE> {
public:
	Handle(HANDLE handle) : zero_or_resource(handle) {}
};

template<typename T>
class OwnableHandle {
public:
	OwnableHandle() = default;
	~OwnableHandle() { CloseHandle(static_cast<T&>(*this)); }
	OwnableHandle(const OwnableHandle&) = delete;
	OwnableHandle& operator=(const OwnableHandle&) = delete;
	OwnableHandle(OwnableHandle&&) noexcept = default;
	OwnableHandle& operator=(OwnableHandle&&) noexcept = default;
};

template<typename T>
class WaitableHandle {
public:
	DWORD WaitFor(DWORD milliseconds = INFINITE) const { return swal::error::throw_or_result(WaitForSingleObject(static_cast<const T&>(*this), milliseconds), wait_func_error_check); }
};

template <typename T>
class EventHandle {
	void Set() const { error::throw_or_result(SetEvent(static_cast<const T&>(*this))); }
	void Reset() const { error::throw_or_result(ResetEvent(static_cast<const T&>(*this))); }
};

enum class EventFlags {
	InitialSet = CREATE_EVENT_INITIAL_SET,
	ManualReset = CREATE_EVENT_MANUAL_RESET
};

template <>
struct enable_enum_bitwise<enum EventFlags> {
	static constexpr bool value = true;
};

class Event : public Handle, public OwnableHandle<Event>, public EventHandle<Event>, public WaitableHandle<Event> {
public:
	Event() noexcept : Handle(NULL) {}
	Event(SECURITY_ATTRIBUTES* sattrs, bool manualReset, bool initialState, tstring_view name)
		: Handle(swal::error::throw_or_result(CreateEvent(sattrs, manualReset, initialState, name.data()))) {}
	Event(bool manualReset, bool initialState)
		: Event(nullptr, manualReset, initialState, tstring_view()) {}
	Event(SECURITY_ATTRIBUTES& sattrs, bool manualReset, bool initialState)
		: Event(&sattrs, manualReset, initialState, tstring_view()) {}
	Event(SECURITY_ATTRIBUTES& sattrs, bool manualReset, bool initialState, tstring_view name)
		: Event(&sattrs, manualReset, initialState, name.data()) {}
	Event(SECURITY_ATTRIBUTES* sattrs, tstring_view name, EventFlags flags, DWORD access)
		: Handle(swal::error::throw_or_result(CreateEventEx(sattrs, name.data(), static_cast<DWORD>(flags), access))) {}
	Event(EventFlags flags, DWORD access)
		: Event(nullptr, tstring_view(), flags, access) {}
	Event(SECURITY_ATTRIBUTES& sattrs, EventFlags flags, DWORD access)
		: Event(&sattrs, tstring_view(), flags, access) {}
	Event(tstring_view name, EventFlags flags, DWORD access)
		: Event(nullptr, name.data(), flags, access) {}
	Event(SECURITY_ATTRIBUTES& sattrs, tstring_view name, EventFlags flags, DWORD access)
		: Event(&sattrs, name.data(), flags, access) {}
};

enum class SetPointerModes {
	Begin = FILE_BEGIN,
	Current = FILE_CURRENT,
	End = FILE_END,
};

template <typename T>
class FileHandle {
private:
	const Handle& handle() const {
		return static_cast<const T&>(*this);
	}
public:
	DWORD Read(LPVOID buffer, DWORD size) const {
		DWORD result;
		error::throw_or_result(ReadFile(handle(), buffer, size, &result, nullptr));
		return result;
	}
	void Read(LPVOID buffer, DWORD size, OVERLAPPED& ovl) const {
		error::throw_or_result(ReadFile(handle(), buffer, size, nullptr, &ovl));
	}
	DWORD GetOverlappedResult(OVERLAPPED& ovl, bool wait = true) const {
		DWORD result;
		error::throw_or_result(::GetOverlappedResult(handle(), &ovl, &result, wait));
		return result;
	}
	DWORD Write(LPVOID buffer, DWORD size) const {
		DWORD result;
		error::throw_or_result(WriteFile(handle(), buffer, size, &result, nullptr));
		return result;
	}
	void Write(LPVOID buffer, DWORD size, OVERLAPPED& ovl) const {
		error::throw_or_result(WriteFile(handle(), buffer, size, nullptr, &ovl));
	}
	LARGE_INTEGER SetPointerEx(LARGE_INTEGER dist, SetPointerModes mode = SetPointerModes::Begin) const {
		LARGE_INTEGER result;
		error::throw_or_result(SetFilePointerEx(handle(), dist, &result, static_cast<DWORD>(mode)));
		return result;
	}
	void SetEndOfFile() const {
		error::throw_or_result(::SetEndOfFile(handle()));
	}
	void CancelIoEx() const {
		error::throw_or_result(::CancelIoEx(handle(), nullptr), CancelIoEx_error_check);
	}
	void CancelIoEx(OVERLAPPED& ovl) const {
		error::throw_or_result(::CancelIoEx(handle(), &ovl), CancelIoEx_error_check);
	}
	LARGE_INTEGER GetSizeEx() const {
		LARGE_INTEGER result;
		error::throw_or_result(GetFileSizeEx(handle(), &result));
		return result;
	}
};

enum class ShareMode {
	Zero = 0,
	Exclusive = 0,
	Read = FILE_SHARE_READ,
	Write = FILE_SHARE_WRITE,
	Delete = FILE_SHARE_DELETE
};

template <>
struct enable_enum_bitwise<enum ShareMode> {
	static constexpr bool value = true;
};

enum class CreateMode {
	CreateNew = CREATE_NEW,
	CreateAlways = CREATE_ALWAYS,
	OpenExisting = OPEN_EXISTING,
	OpenAlways = OPEN_ALWAYS,
	TruncateExisting = TRUNCATE_EXISTING
};

class File : public Handle, public FileHandle<File>, public OwnableHandle<File>, public WaitableHandle<File> {
public:
	File() noexcept : Handle(INVALID_HANDLE_VALUE) {}
	File(tstring_view filename, DWORD access, ShareMode shareMode, SECURITY_ATTRIBUTES* secattrs, CreateMode createMode, DWORD flags, HANDLE tmplt)
		: Handle(error::throw_or_result(CreateFile(filename.data(), access, static_cast<DWORD>(shareMode), secattrs, static_cast<DWORD>(createMode), flags, tmplt), CreateFile_error_check)) {}
	File(tstring_view filename, DWORD access, ShareMode shareMode, SECURITY_ATTRIBUTES& secattrs, CreateMode createMode, DWORD flags, const Handle& tmplt)
		: File(filename.data(), access, shareMode, &secattrs, createMode, flags, tmplt) {}
	File(tstring_view filename, DWORD access, ShareMode shareMode, SECURITY_ATTRIBUTES& secattrs, CreateMode createMode, DWORD flags)
		: File(filename.data(), access, shareMode, &secattrs, createMode, flags, NULL) {}
	File(tstring_view filename, DWORD access, ShareMode shareMode, CreateMode createMode, DWORD flags, const Handle& tmplt)
		: File(filename.data(), access, shareMode, nullptr, createMode, flags, tmplt) {}
	File(tstring_view filename, DWORD access, ShareMode shareMode, CreateMode createMode, DWORD flags)
		: File(filename.data(), access, shareMode, nullptr, createMode, flags, NULL) {}
};

struct CompletionStatusResult {
	DWORD error;
	DWORD bytesTransfered;
	ULONG_PTR key;
	OVERLAPPED* ovl;
};

template <typename T>
class IOCompletionPortHandle {
private:
	const Handle& handle() const {
		return static_cast<const T&>(*this);
	}
public:
	void AssocFile(const Handle& file, ULONG_PTR key) const {
		error::throw_or_result(CreateIoCompletionPort(file, handle(), key, 0));
	}
	CompletionStatusResult GetQueuedCompletionStatus(DWORD timeout) const {
		CompletionStatusResult result;
		result.error = ERROR_SUCCESS;
		if (!::GetQueuedCompletionStatus(handle(), &result.bytesTransfered, &result.key, &result.ovl, timeout)) {
			if (!result.ovl) {
				error::throw_last_error();
			} else {
				result.error = GetLastError();
			}
		}
		return result;
	}
	void PostQueuedCompletionStatus(DWORD transfered, ULONG_PTR key, OVERLAPPED* ovl) const {
		error::throw_or_result(::PostQueuedCompletionStatus(handle(), transfered, key, ovl));
	}
};

class IOCompletionPort : public Handle, public IOCompletionPortHandle<IOCompletionPort>, public OwnableHandle<IOCompletionPort> {
public:
	IOCompletionPort(DWORD thrNum = 0)
		: Handle(error::throw_or_result(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thrNum))) {}
	IOCompletionPort(const Handle& file, ULONG_PTR key, DWORD thrNum = 0)
		: Handle(error::throw_or_result(CreateIoCompletionPort(file, NULL, key, thrNum))) {}
};

}

#endif /* WIN32_HANDLE_H_ */
