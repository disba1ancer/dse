/*
 * strconv.h
 *
 *  Created on: 29 авг. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_STRCONV_H
#define SWAL_STRCONV_H

#include "win_headers.h"
#include <string>

namespace swal {

#ifdef UNICODE
typedef std::wstring tstring;
typedef std::wstring_view tstring_view;
#else
typedef std::string tstring;
typedef std::string_view tstring_view;
#endif

template <typename T>
concept ConvertionReceiverString =
	std::same_as<T, std::string> ||
	std::same_as<T, std::u8string>;

template <ConvertionReceiverString T>
inline T basic_wide_char_to_multibyte(
	UINT cp, DWORD flags,
	std::wstring_view string,
	char* defaultChar, BOOL* usedDefaultChar
) {
	T result;
	result.resize(std::size_t(
		WideCharToMultiByte(
			cp, flags,
			string.data(), int(string.size()),
			nullptr, 0,
			defaultChar, usedDefaultChar
		)
	));
	WideCharToMultiByte(
		cp, flags,
		string.data(), int(string.size()),
		reinterpret_cast<char*>(result.data()), int(result.size()),
		defaultChar, usedDefaultChar
	);
	return result;
}

inline std::string wide_char_to_multibyte(
	UINT cp, DWORD flags,
	std::wstring_view string,
	char* defaultChar, BOOL* usedDefaultChar
) {
	return basic_wide_char_to_multibyte<std::string>(
		cp, flags,
		string,
		defaultChar, usedDefaultChar
	);
}

inline std::string wide_char_to_multibyte(UINT cp, std::wstring_view string) {
	return wide_char_to_multibyte(cp, 0, string, nullptr, nullptr);
}

inline std::string wide_char_to_multibyte(UINT cp, DWORD flags, std::wstring_view string) {
	return wide_char_to_multibyte(cp, flags, string, nullptr, nullptr);
}

inline std::string wide_char_to_multibyte(UINT cp, DWORD flags, std::wstring_view string, char defaultChar) {
	return wide_char_to_multibyte(cp, flags, string, &defaultChar, nullptr);
}

inline std::string wide_char_to_multibyte(UINT cp, DWORD flags, std::wstring_view string, char defaultChar, BOOL& usedDefaultChar) {
	return wide_char_to_multibyte(cp, flags, string, &defaultChar, &usedDefaultChar);
}

inline std::u8string wide_char_to_multibyte8(
	UINT cp, DWORD flags,
	std::wstring_view string,
	char* defaultChar, BOOL* usedDefaultChar
) {
	return basic_wide_char_to_multibyte<std::u8string>(
		cp, flags,
		string,
		defaultChar, usedDefaultChar
	);
}

inline std::u8string wide_char_to_multibyte8(UINT cp, std::wstring_view string) {
	return wide_char_to_multibyte8(cp, 0, string, nullptr, nullptr);
}

inline std::u8string wide_char_to_multibyte8(UINT cp, DWORD flags, std::wstring_view string) {
	return wide_char_to_multibyte8(cp, flags, string, nullptr, nullptr);
}

inline std::u8string wide_char_to_multibyte8(UINT cp, DWORD flags, std::wstring_view string, char defaultChar) {
	return wide_char_to_multibyte8(cp, flags, string, &defaultChar, nullptr);
}

inline std::u8string wide_char_to_multibyte8(UINT cp, DWORD flags, std::wstring_view string, char defaultChar, BOOL& usedDefaultChar) {
	return wide_char_to_multibyte8(cp, flags, string, &defaultChar, &usedDefaultChar);
}

inline std::wstring multibyte_to_wide_char(UINT cp, DWORD flags, std::string_view string) {
	std::wstring result;
	result.resize(std::size_t(
		MultiByteToWideChar(
			cp, flags,
			string.data(), int(string.size()),
			nullptr, 0
		)
	));
	MultiByteToWideChar(
		cp, flags,
		string.data(), int(string.size()),
		result.data(), int(result.size())
	);
	return result;
}

inline std::wstring multibyte_to_wide_char(UINT cp, std::string_view string) {
	return multibyte_to_wide_char(cp, 0, string);
}

inline std::wstring multibyte_to_wide_char(UINT cp, DWORD flags, std::u8string_view string) {
	return multibyte_to_wide_char(
		cp, flags, { reinterpret_cast<const char*>(string.data()), string.size() }
	);
}

inline std::wstring multibyte_to_wide_char(UINT cp, std::u8string_view string) {
	return multibyte_to_wide_char(cp, 0, string);
}

inline std::u8string wide_char_to_u8(std::wstring_view str) {
	return wide_char_to_multibyte8(CP_UTF8, str);
}

inline std::wstring u8_to_wide_char(std::u8string_view str) {
	return multibyte_to_wide_char(CP_UTF8, str);
}

inline std::u8string u8fromTString(tstring_view str) {
#ifdef UNICODE
	return wide_char_to_u8(str);
#else
	return std::u8string(reinterpret_cast<const char8_t*>(str.data()), str.size());
#endif
}

#ifdef UNICODE
inline std::string fromTString(std::wstring_view str) {
	return wide_char_to_multibyte(CP_UTF8, str);
}
#else
inline std::string fromTString(std::string str) {
	return str;
}
#endif

}

#endif /* SWAL_STRCONV_H */
