/*
 * WindowData_win32.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_WINDOW_WIN32_H_
#define DSE_CORE_WINDOW_WIN32_H_

#include "detail/Window.h"

namespace dse::util {

template <core::WindowEvent evt>
struct event_traits<evt> {
    using handler = void(HWND, WPARAM, LPARAM);
};

template <>
struct event_traits<core::WindowEvent::System + WM_ERASEBKGND> {
    using handler = void(HWND, HDC, LRESULT&);
};

}

namespace dse::core {

struct WindowData_win32 {
	HWND hWnd;
};

struct WindowEventData_win32
{
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

} // namespace dse::core

#endif /* DSE_CORE_WINDOW_WIN32_H_ */
