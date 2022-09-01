/*
 * mcursor.h
 *
 *  Created on: 25 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_MCURSOR_H_
#define DSE_CORE_MCURSOR_H_

#include <dse/math/vec.h>
#include "Window.h"
#include "detail/impexp.h"

namespace dse::core {

void API_DSE_CORE setMouseCursorPos(const math::ivec2& pos);
void API_DSE_CORE SetMouseCursorPosWndRel(const math::ivec2& pos, Window& wnd);

} // namespace dse::core

#endif /* DSE_CORE_MCURSOR_H_ */
