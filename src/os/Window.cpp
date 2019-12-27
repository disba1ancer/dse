/*
 * Window.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: Anton
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

} /* namespace os */
} /* namespace dse */
