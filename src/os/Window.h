/*
 * Window.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_WINDOW_H_
#define OS_WINDOW_H_

#include <memory>
#include "WindowShowCommand.h"
#include "../notifier/notifier.h"

namespace dse {
namespace os {

class Window_win32;

typedef Window_win32 Window_impl;

class Window {
	std::unique_ptr<Window_impl> impl;
public:
	Window();
	~Window();
	Window(const Window &other) = delete;
	Window(Window &&other) = delete;
	Window& operator=(const Window &other) = delete;
	Window& operator=(Window &&other) = delete;
	bool isVisible() const;
	void show(WindowShowCommand command = WindowShowCommand::SHOW);
	notifier::connection<void()> subscribeCloseEvent(std::function<void()>&& c);
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_WINDOW_H_ */
