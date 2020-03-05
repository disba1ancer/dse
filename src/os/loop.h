/*
 * loop.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_LOOP_H_
#define OS_LOOP_H_

#include <functional>

namespace dse {
namespace os {

std::function<bool()> nonLockLoop();

}
}

#endif /* OS_LOOP_H_ */
