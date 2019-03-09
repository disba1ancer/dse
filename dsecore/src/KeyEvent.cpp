/*
 * KeyEvent.cpp
 *
 *  Created on: 18 янв. 2019 г.
 *      Author: Anton
 */

#include <dse/KeyEvent.h>
#include <dse/Terminal.h>

namespace dse {
namespace core {

KeyEvent::KeyEvent(State state, unsigned long key)
: state(state), key(key) {}

long KeyEvent::getEventID() const {
	return Terminal::EVENT_ON_KEY_INPUT;
}

} /* namespace core */
} /* namespace dse */
