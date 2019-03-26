/*
 * RedrawEvent.cpp
 *
 *  Created on: 17 мар. 2019 г.
 *      Author: Anton
 */

#include "../dse/RedrawEvent.h"
#include "../dse/Terminal.h"

namespace dse {
namespace core {

long dse::core::RedrawEvent::getEventID() const {
	return Terminal::EVENT_ON_REDRAW;
}

} /* namespace core */
} /* namespace dse */
