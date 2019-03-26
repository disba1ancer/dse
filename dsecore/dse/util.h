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
 * dse_util.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_UTIL_H_
#define DSE_UTIL_H_

#include <cinttypes>
#include <type_traits>

#include "utemplate.h"

namespace dse {
namespace util {

int mainLoop(void (*func)(const void *) = nullptr, const void *data = nullptr);

template <typename T>
int mainLoop(const T& func) {
	return mainLoop([](const void *data){ (*static_cast<typename std::remove_reference<const T>::type*>(data))(); }, &func);
}

/*template <typename T>
int mainLoop(T& func) {
	return mainLoop([](void *data){ (*static_cast<T*>(data))(); }, &func);
}*/

void returnMainLoop(int returnValue);

}
}

#endif /* DSE_UTIL_H_ */
