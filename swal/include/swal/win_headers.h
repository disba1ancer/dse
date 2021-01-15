#ifndef WIN_HEADERS_H
#define WIN_HEADERS_H

#ifndef _WIN32_WINNT
#warning _WIN32_WINNT is not set, set it in your build system
#endif
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>

#endif // WIN_HEADERS_H
