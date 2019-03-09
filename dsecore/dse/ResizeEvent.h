/*
 * ResizeEvent.h
 *
 *  Created on: 11 нояб. 2018 г.
 *      Author: Anton
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
