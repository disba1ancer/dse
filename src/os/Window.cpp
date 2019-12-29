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

notifier::connection<Window::CloseHandler> Window::subscribeCloseEvent(
		std::function<CloseHandler>&& c) {
	return impl->subscribeCloseEvent(std::move(c));
}

notifier::connection<Window::ResizeHandler> Window::subscribeResizeEvent(
		std::function<ResizeHandler>&& c) {
	return impl->subscribeResizeEvent(std::move(c));
}

notifier::connection<Window::KeyHandler> Window::subscribeKeyEvent(
		std::function<KeyHandler> &&c) {
	return impl->subscribeKeyEvent(std::move(c));
}

} /* namespace os */
} /* namespace dse */
