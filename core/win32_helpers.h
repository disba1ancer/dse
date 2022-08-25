/*
 * win32_helpers.h
 *
 *  Created on: 29 июн. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_WIN32_HELPERS_H_
#define OS_WIN32_HELPERS_H_

#include <string>
#include "win32.h"

namespace dse {
namespace os {
namespace win32_helpers {

std::u8string getErrorString(DWORD error);

}
}
}

#endif /* OS_WIN32_HELPERS_H_ */
