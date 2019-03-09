/*
 * KeyEvent.h
 *
 *  Created on: 18 янв. 2019 г.
 *      Author: Anton
 */

#ifndef KEYEVENT_H_
#define KEYEVENT_H_

#include "Event.h"

namespace dse {
namespace core {

class KeyEvent: public Event {
public:
	enum State {
		UP,
		DOWN,
		PRESSED
	};

	KeyEvent(State state, unsigned long key);
	long getEventID() const override;

	const State state;
	const unsigned long key;
};

} /* namespace core */
} /* namespace dse */

#endif /* KEYEVENT_H_ */
