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

Window::Window(SystemLoop& loop) : impl(loop)
{}

Window::~Window() = default;

bool Window::IsVisible() const
{
	return impl->IsVisible();
}

void Window::Show(WindowShowCommand command)
{
	return impl->Show(command);
}

const WindowData& Window::GetSysData()
{
	return impl->GetSysData();
}

math::ivec2 Window::Size()
{
	return impl->Size();
}

void Window::Resize(const math::ivec2& size)
{
	impl->Resize(size);
}

void Window::ChangeFrameStyle(WindowFrameStyle style)
{
	impl->ChangeFrameStyle(style);
}

bool Window::HasMinimizeCtl()
{
	return impl->HasMinimizeCtl();
}

void Window::ShowMinimizeCtl(bool state)
{
	impl->ShowMinimizeCtl(state);
}

auto Window::GetLoop() const -> SystemLoop&
{
	return impl->GetLoop();
}

void Window::SetTitle(const char8_t* title)
{
	impl->SetTitle(title);
}

notifier::connection<Window::CloseHandler> Window::SubscribeCloseEvent(
		std::function<CloseHandler>&& c)
{
	return impl->SubscribeCloseEvent(std::move(c));
}

notifier::connection<Window::ResizeHandler> Window::SubscribeResizeEvent(
		std::function<ResizeHandler>&& c)
{
	return impl->SubscribeResizeEvent(std::move(c));
}

notifier::connection<Window::KeyHandler> Window::SubscribeKeyEvent(
		std::function<KeyHandler> &&c)
{
	return impl->SubscribeKeyEvent(std::move(c));
}

notifier::connection<Window::PaintHandler> Window::SubscribePaintEvent(
		std::function<PaintHandler> &&c)
{
	return impl->SubscribePaintEvent(std::move(c));
}

notifier::connection<Window::MouseMoveHandler> Window::SubscribeMouseMoveEvent(
		std::function<MouseMoveHandler> &&c)
{
	return impl->SubscribeMouseMoveEvent(std::move(c));
}

} /* namespace dse::core */
