/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 disba1ancer.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * KeyEvent.h
 *
 *  Created on: 18 янв. 2019 г.
 *      Author: disba1ancer
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
