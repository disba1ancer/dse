/*
 * ResizeEvent.cpp
 *
 *  Created on: 11 нояб. 2018 г.
 *      Author: Anton
 */

#include <dse/ResizeEvent.h>
#include <dse/Terminal.h>

namespace dse {
namespace core {

ResizeEvent::ResizeEvent(int newWidth, int newHeight) : width(newWidth), height(newHeight) {
}

long ResizeEvent::getEventID() const {
	return Terminal::EVENT_ON_RESIZE;
}

} /* namespace core */
} /* namespace dse */
