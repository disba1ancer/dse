/*
 * win32_error.h
 *
 *  Created on: 28 авг. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_ERROR_H
#define SWAL_ERROR_H

#include "win_headers.h"
//#include <wchar.h>
#include <exception>
#include <string>
#include <memory>
#include <type_traits>
#include <system_error>
#include "strconv.h"

namespace swal {
enum class win32_errc : DWORD {};
enum class com_errc : HRESULT {};
}
namespace std {
template <> struct is_error_code_enum<swal::win32_errc> : true_type {};
template <> struct is_error_code_enum<swal::com_errc> : true_type {};
}

namespace swal {

inline tstring get_error_string(DWORD error) {
	constexpr std::size_t resultStringMaxSize = 512;
	TCHAR wStr[resultStringMaxSize];
	auto wSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error, 0, wStr, resultStringMaxSize, nullptr);
	return tstring(wStr, wSize);
}

class win32_category : public std::error_category {
	win32_category() = default;
public:
	const char* name() const noexcept override {
		return "Win32 error";
	}
	std::string message(int condition) const override {
		return swal::fromTString(get_error_string(DWORD(condition)));
	}
	static const win32_category& instance() {
		static win32_category instance;
		return instance;
	}
};

class com_category : public std::error_category {
	com_category() = default;
public:
	const char* name() const noexcept override {
		return "COM error";
	}
	std::string message(int condition) const override {
		return swal::fromTString(_com_error(HRESULT(condition)).ErrorMessage());
	}
	static const com_category& instance() {
		static com_category instance;
		return instance;
	}
};

inline std::error_code make_error_code(win32_errc err) {
	return { int(DWORD(err)), win32_category::instance() };
}

inline std::error_code make_error_code(com_errc err) {
	return { int(HRESULT(err)), com_category::instance() };
}

template <typename T>
T winapi_call(T result) {
	if (!result) {
		auto err = GetLastError();
		throw std::system_error(win32_errc(err));
	}
	return result;
}

template <typename T, typename F>
typename std::remove_reference<F>::type winapi_call(T result, DWORD(*chk)(F)) {
	DWORD err = chk(result);
	if (err != ERROR_SUCCESS) {
		throw std::system_error(win32_errc(err));
	}
	return result;
}

template <typename T, typename F>
T winapi_call(T result, const F& chk) {
	DWORD err = chk(result);
	if (err != ERROR_SUCCESS) {
		throw std::system_error(win32_errc(err));
	}
	return result;
}

inline HRESULT com_call(HRESULT result) {
	if (FAILED(result)) {
		throw std::system_error(com_errc(result));
	}
	return result;
}

inline auto last_error() -> std::error_code {
	return win32_errc(GetLastError());
}

inline void throw_last_error() {
	auto err = last_error();
	if (err) {
		throw err;
	}
}

inline DWORD GetMessage_error_check(BOOL result) {
	return ((result != -1) ? ERROR_SUCCESS : GetLastError());
}

inline DWORD wait_func_error_check(DWORD result) {
	return ((result != WAIT_FAILED) ? ERROR_SUCCESS : GetLastError());
}

inline DWORD GetWindowLongPtr_error_check(LONG_PTR result) {
	return (result ? ERROR_SUCCESS : GetLastError());
}

inline DWORD invalid_color_error_check(COLORREF result) {
	return (result != CLR_INVALID ? ERROR_SUCCESS : GetLastError());
}

inline DWORD ReadWriteFile_error_check(DWORD& result) {
	if (!result) {
		result = GetLastError();
		switch (result) {
		case ERROR_IO_PENDING:
		case ERROR_HANDLE_EOF:
			return 0;
		}
	} else {
		result = ERROR_SUCCESS;
	}
	return result;
}

inline DWORD CancelIoEx_error_check(BOOL result) {
	if (!result) {
		auto err = GetLastError();
		if (err != ERROR_NOT_FOUND) {
			return err;
		}
	}
	return ERROR_SUCCESS;
}

inline DWORD CreateFile_error_check(HANDLE result) {
	if (result == INVALID_HANDLE_VALUE) {
		return GetLastError();
	}
	return ERROR_SUCCESS;
}

}

#endif /* SWAL_ERROR_H */
