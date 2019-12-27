/*
 * WindowShowCommand.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: Anton
 */

#ifndef OS_WINDOWSHOWCOMMAND_H_
#define OS_WINDOWSHOWCOMMAND_H_

namespace dse {
namespace os {

enum class WindowShowCommand {
	HIDE,
	SHOW,
	SHOW_MINIMIZED,
	SHOW_RESTORED,
	SHOW_MAXIMIZED,
	SHOW_FULL_SCREEN
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_WINDOWSHOWCOMMAND_H_ */
