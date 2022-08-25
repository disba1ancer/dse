/*
 * WindowData_win32.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_WINDOWDATA_WIN32_H_
#define DSE_CORE_WINDOWDATA_WIN32_H_

#include "win32.h"

namespace dse::core {

struct WindowData_win32 {
	HWND hWnd;
};

} // namespace dse::core

#endif /* DSE_CORE_WINDOWDATA_WIN32_H_ */
