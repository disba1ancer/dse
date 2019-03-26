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
 * Event.h
 *
 *  Created on: 1 окт. 2018 г.
 *      Author: disba1ancer
 */

#ifndef EVENT_H_
#define EVENT_H_

namespace dse {
namespace core {

struct Event {
	virtual long getEventID() const = 0;
protected:
	~Event() = default;
};

} /* namespace core */
} /* namespace dse */

#endif /* EVENT_H_ */
