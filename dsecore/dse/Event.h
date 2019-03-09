/*
 * Event.h
 *
 *  Created on: 1 окт. 2018 г.
 *      Author: Anton
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
