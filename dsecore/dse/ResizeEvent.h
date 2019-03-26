/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 ${user}.
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
 * ResizeEvent.h
 *
 *  Created on: 11 нояб. 2018 г.
 *      Author: disba1ancer
 */

#ifndef RESIZEEVENT_H_
#define RESIZEEVENT_H_

#include "Event.h"

namespace dse {
namespace core {

struct ResizeEvent: public Event {
	ResizeEvent(int newWidth, int newHeight);
	long getEventID() const override;

	const int width, height;
};

} /* namespace dse */
} /* namespace dse */

#endif /* RESIZEEVENT_H_ */
