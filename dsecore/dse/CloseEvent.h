/*
 * CloseEvent.h
 *
 *  Created on: 3 окт. 2018 г.
 *      Author: Anton
 */

#ifndef CLOSEEVENT_H_
#define CLOSEEVENT_H_

#include "Event.h"

namespace dse {
namespace core {

class CloseEvent: public Event {
public:
	long getEventID() const override;
};

} /* namespace core */
} /* namespace dse */

#endif /* CLOSEEVENT_H_ */
