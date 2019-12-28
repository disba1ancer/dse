/*
 * Window.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "Window.h"
#include "Window_win32.h"

namespace dse {
namespace os {

Window::Window() : impl(new Window_impl()) {
}

Window::~Window() {
}

bool Window::isVisible() const {
	return impl->isVisible();
}

void Window::show(WindowShowCommand command) {
	return impl->show(command);
}

notifier::connection<void()> Window::subscribeCloseEvent(
		std::function<void()>&& c) {
	return impl->subscribeCloseEvent(std::move(c));
}

} /* namespace os */
} /* namespace dse */
