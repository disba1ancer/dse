/*
 * Window.cpp
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifdef _WIN32
#include "Window_win32.h"
#include <dse/core/Window_win32.h>
#endif

#include <dse/core/Window.h>

namespace dse::core {

Window::Window(SystemLoop& loop) : impl(loop)
{}

Window::~Window()
{}

bool Window::IsVisible() const
{
	return impl->IsVisible();
}

bool Window::IsFullscreen() const
{
	return impl->IsFullscreen();
}

void Window::Show(WindowShowCommand command)
{
	return impl->Show(command);
}

auto Window::GetSysData() -> WindowData
{
	return impl->GetSysData();
}

auto Window::Position() const -> math::ivec2
{
	return impl->Position();
}

void Window::Move(const math::ivec2& pos)
{
	impl->Move(pos);
}

auto Window::Size() const -> math::ivec2
{
	return impl->Size();
}

void Window::Resize(const math::ivec2& size)
{
	impl->Resize(size);
}

auto Window::SurfaceSize() const -> math::ivec2
{
	return impl->SurfaceSize();
}

void Window::ResizeSurface(const math::ivec2& size)
{
	impl->ResizeSurface(size);
}

void Window::SetTitle(const char8_t* title)
{
	impl->SetTitle(title);
}

void Window::ChangeFrameStyle(WindowFrameStyle style)
{
	impl->ChangeFrameStyle(style);
}

bool Window::Minimizable() const
{
	return impl->Minimizable();
}

void Window::MakeMinimizable(bool state)
{
	impl->MakeMinimizable(state);
}

auto Window::GetLoop() const -> SystemLoop&
{
	return impl->GetLoop();
}

auto Window::SubscribeEvent(WindowEvent evt, void* object, void (*cb)()) -> std::size_t
{
	return impl->Register(evt, object, cb);
}

void Window::UnsubscribeEvent(std::size_t id) noexcept
{
    return impl->Unregister(id);
}

} /* namespace dse::core */
