/*
 * PaintEventData_win32.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_PAINTEVENTDATA_WIN32_H_
#define DSE_CORE_PAINTEVENTDATA_WIN32_H_

#include "win32.h"

namespace dse::core {

struct PaintEventData_win32 {
HDC hdc;
};

} /* namespace dse::core */

#endif /* DSE_CORE_PAINTEVENTDATA_WIN32_H_ */
