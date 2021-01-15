/*
 * loop.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_LOOP_H_
#define OS_LOOP_H_

#include <functional>
#include "util/TaskState.h"

namespace dse {
namespace os {

std::function<util::TaskState()> nonLockLoop();

}
}

#endif /* OS_LOOP_H_ */
