/*
 * Window.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifdef _WIN32
#include "Window_win32.h"
#endif

#include <dse/core/Window.h>

namespace dse::core {

Window::Window(UILoop& loop) : impl(loop)
{}

Window::~Window() = default;

bool Window::IsVisible() const
{
	return impl->isVisible();
}

void Window::Show(WindowShowCommand command)
{
	return impl->show(command);
}

const WindowData& Window::GetSysData()
{
	return impl->getSysData();
}

math::ivec2 Window::Size()
{
	return impl->size();
}

void Window::Resize(const math::ivec2& size)
{
	impl->resize(size);
}

notifier::connection<Window::CloseHandler> Window::SubscribeCloseEvent(
		std::function<CloseHandler>&& c)
{
	return impl->subscribeCloseEvent(std::move(c));
}

notifier::connection<Window::ResizeHandler> Window::SubscribeResizeEvent(
		std::function<ResizeHandler>&& c)
{
	return impl->subscribeResizeEvent(std::move(c));
}

notifier::connection<Window::KeyHandler> Window::SubscribeKeyEvent(
		std::function<KeyHandler> &&c)
{
	return impl->subscribeKeyEvent(std::move(c));
}

notifier::connection<Window::PaintHandler> Window::SubscribePaintEvent(
		std::function<PaintHandler> &&c)
{
	return impl->subscribePaintEvent(std::move(c));
}

notifier::connection<Window::MouseMoveHandler> Window::SubscribeMouseMoveEvent(
		std::function<MouseMoveHandler> &&c)
{
	return impl->subscribeMouseMoveEvent(std::move(c));
}

} /* namespace dse::core */
