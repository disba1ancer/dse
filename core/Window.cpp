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

const WindowData& Window::GetSysData()
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

bool Window::Register(WindowEvent evt, void* object, void (*cb)())
{
	return impl->Register(evt, object, cb);
}

void Window::Unregister(WindowEvent evt, void* object, void (*cb)()) noexcept
{
	return impl->Unregister(evt, object, cb);
}

} /* namespace dse::core */
