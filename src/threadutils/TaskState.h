/*
 * TaskState.h
 *
 *  Created on: 30 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef THREADUTILS_TASKSTATE_H_
#define THREADUTILS_TASKSTATE_H_

namespace dse {
namespace threadutils {

enum class TaskState {
	END,
	AWAIT,
	YIELD
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADUTILS_TASKSTATE_H_ */
