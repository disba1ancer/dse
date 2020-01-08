/*
 * PaintEventData_win32.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_PAINTEVENTDATA_WIN32_H_
#define OS_PAINTEVENTDATA_WIN32_H_

#include "win32.h"

namespace dse {
namespace os {

struct PaintEventData_win32 {
HDC hdc;
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_PAINTEVENTDATA_WIN32_H_ */
