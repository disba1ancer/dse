/*
 * RedrawEvent.h
 *
 *  Created on: 17 мар. 2019 г.
 *      Author: Anton
 */

#ifndef DSE_REDRAWEVENT_H_
#define DSE_REDRAWEVENT_H_

#include "Event.h"

namespace dse {
namespace core {

class RedrawEvent: public Event {
	long getEventID() const override;
};

} /* namespace core */
} /* namespace dse */

#endif /* DSE_REDRAWEVENT_H_ */
