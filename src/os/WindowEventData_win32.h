/*
 * WindowEventData_win32.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_WINDOWEVENTDATA_WIN32_H_
#define OS_WINDOWEVENTDATA_WIN32_H_

#include "win32.h"

namespace dse {
namespace os {

struct WindowEventData_win32 {
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_WINDOWEVENTDATA_WIN32_H_ */
