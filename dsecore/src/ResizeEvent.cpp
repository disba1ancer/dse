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
 * ResizeEvent.cpp
 *
 *  Created on: 11 нояб. 2018 г.
 *      Author: disba1ancer
 */

#include "../dse/ResizeEvent.h"
#include "../dse/Terminal.h"

namespace dse {
namespace core {

ResizeEvent::ResizeEvent(int newWidth, int newHeight) : width(newWidth), height(newHeight) {
}

long ResizeEvent::getEventID() const {
	return Terminal::EVENT_ON_RESIZE;
}

} /* namespace core */
} /* namespace dse */
