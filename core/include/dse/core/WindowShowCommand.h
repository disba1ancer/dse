/*
 * WindowShowCommand.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_WINDOWSHOWCOMMAND_H_
#define DSE_CORE_WINDOWSHOWCOMMAND_H_

#include <dse/util/enum_bitwise.h>

namespace dse::core {

enum class WindowShowCommand {
	Hide,
	Show,
	ShowMinimized,
	ShowRestored,
	ShowMaximized,
	ShowFullScreen
};

enum class WindowFrameStyle {
	None,
	Fixed,
	Sizable
};

} /* namespace dse::core */

#endif /* DSE_CORE_WINDOWSHOWCOMMAND_H_ */
