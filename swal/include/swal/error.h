/*
 * win32_error.h
 *
 *  Created on: 28 авг. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_ERROR_H_
#define WIN32_ERROR_H_

#include <windows.h>
#include <wchar.h>
#include <comdef.h>
#include <exception>
#include <string>
#include <memory>
#include <type_traits>
#include "strconv.h"

namespace swal {

class error : public std::exception {
public:

	template <typename T>
	static T throw_or_result(T result) {
		if (!result) throw error(GetLastError());
		return result;
	}

	template <typename T, typename F>
	static typename std::remove_reference<F>::type throw_or_result(T result, DWORD(*chk)(F)) {
		DWORD err = chk(result);
		if (err != ERROR_SUCCESS) throw error(err);
		return result;
	}

	template <typename T, typename F>
	static T throw_or_result(T result, const F& chk) {
		DWORD err = chk(result);
		if (err != ERROR_SUCCESS) throw error(err);
		return result;
	}

	static void throw_last_error() {
		auto err = GetLastError();
		if (err != ERROR_SUCCESS) throw error(err);
	}

	static tstring get_error_string(DWORD error) {
		constexpr std::size_t resultStringMaxSize = 512;
		TCHAR wStr[resultStringMaxSize];
		auto wSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error, 0, wStr, resultStringMaxSize, nullptr);
		return tstring(wStr, wSize);
	}

	error(DWORD error) noexcept : m_error(error), errStr(std::make_shared<std::u8string>(wide_char_to_u8(get_error_string(error)))) {}

	virtual const char* what() const noexcept override {
		return reinterpret_cast<const char*>(errStr->c_str());
	}

	DWORD get() {
		return m_error;
	}
private:
	DWORD m_error;
	std::shared_ptr<std::u8string> errStr;
};

class com_error : public std::exception {
public:

	static HRESULT throw_or_result(HRESULT result) {
		if (FAILED(result)) throw com_error(result);
		return result;
	}

	/*template <typename F>
	static HRESULT throw_or_result(HRESULT result, const F& chk) {
		if (!chk(result)) throw com_error(result);
		return result;
	}*/

	com_error(HRESULT error) noexcept : error(error), errStr(std::make_shared<std::u8string>(wide_char_to_u8(_com_error(error).ErrorMessage()))) {}

	virtual const char* what() const noexcept override {
		return reinterpret_cast<const char*>(errStr->c_str());
	}

	HRESULT get() {
		return error;
	}
private:
	HRESULT error;
	std::shared_ptr<std::u8string> errStr;
};

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

#endif /* WIN32_ERROR_H_ */
