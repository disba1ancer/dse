/*
 * mcursor.h
 *
 *  Created on: 25 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_MCURSOR_H_
#define OS_MCURSOR_H_

#include "math/vec.h"
#include "Window.h"

namespace dse {
namespace os {

void setMouseCursorPos(const math::ivec2& pos);
void SetMouseCursorPosWndRel(const math::ivec2& pos, Window& wnd);

} // namespace os
} // namespace dse

#endif /* OS_MCURSOR_H_ */
