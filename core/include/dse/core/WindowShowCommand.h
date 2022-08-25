/*
 * WindowShowCommand.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_WINDOWSHOWCOMMAND_H_
#define DSE_CORE_WINDOWSHOWCOMMAND_H_

namespace dse::core {

enum class WindowShowCommand {
	Hide,
	Show,
	ShowMinimized,
	ShowRestored,
	ShowMaximized,
	ShowFullScreen
};

} /* namespace dse::core */

#endif /* DSE_CORE_WINDOWSHOWCOMMAND_H_ */
