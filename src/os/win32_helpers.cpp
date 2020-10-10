/*
 * win32.cpp
 *
 *  Created on: 29 июн. 2020 г.
 *      Author: disba1ancer
 */

#include "win32_helpers.h"

namespace {

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
		result += std::u8string_view(u8code, c32tou8(u8code, code));
	}
	return result;
}

}

namespace dse {
namespace os {
namespace win32_helpers {

std::u8string getErrorString(DWORD error) {
	constexpr std::size_t resultStringMaxSize = 512;
	wchar_t buf[resultStringMaxSize];
	auto size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error, 0, buf, resultStringMaxSize, nullptr);
	return wcTou8(std::wstring_view(buf, size));
}

}
}
}
