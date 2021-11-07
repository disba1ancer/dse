/*
 * WindowShowCommand.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_WINDOWSHOWCOMMAND_H_
#define OS_WINDOWSHOWCOMMAND_H_

namespace dse {
namespace os {

enum class WindowShowCommand {
	Hide,
	Show,
	ShowMinimized,
	ShowRestored,
	ShowMaximized,
	ShowFullScreen
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_WINDOWSHOWCOMMAND_H_ */
