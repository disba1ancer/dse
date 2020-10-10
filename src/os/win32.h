/*
 * win32.h
 *
 *  Created on: 29 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_WIN32_H_
#define OS_WIN32_H_

#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <windowsx.h>

#endif /* OS_WIN32_H_ */
