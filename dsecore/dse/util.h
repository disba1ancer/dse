/*
 * dse_util.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: Anton
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
