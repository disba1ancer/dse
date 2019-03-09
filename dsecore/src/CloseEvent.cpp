/*
 * CloseEvent.cpp
 *
 *  Created on: 3 окт. 2018 г.
 *      Author: Anton
 */

#include <dse/Terminal.h>
#include <dse/CloseEvent.h>

namespace dse {
namespace core {

/*CloseEvent::CloseEvent() {}

CloseEvent::~CloseEvent() {}*/

long CloseEvent::getEventID() const {
	return Terminal::EVENT_ON_CLOSE;
}

} /* namespace core */
} /* namespace dse */
