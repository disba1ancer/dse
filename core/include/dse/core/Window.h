/*
 * Window.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_WINDOW_H_
#define DSE_CORE_WINDOW_H_

#include "detail/Window.h"
#include <dse/notifier/notifier.h>
#include <dse/math/vec.h>
#include "detail/impexp.h"
#include <dse/core/SystemLoop.h>
#include <dse/util/pimpl.h>

namespace dse::core {

#ifdef _WIN32

class Window_win32;
struct WindowData_win32;
struct WindowEventData_win32;
struct PaintEventData_win32;

typedef Window_win32 Window_impl;
typedef WindowData_win32 WindowData;
typedef WindowEventData_win32 WindowEventData;
typedef PaintEventData_win32 PaintEventData;

#endif

typedef const WindowEventData& WndEvtDt;
typedef const PaintEventData& PntEvtDt;

class API_DSE_CORE Window {
public:
    Window(SystemLoop& loop);
	~Window();
	Window(Window &&other) = delete;
	Window(const Window &other) = delete;
	Window& operator=(Window &&other) = delete;
	Window& operator=(const Window &other) = delete;
	bool IsVisible() const;
	bool IsFullscreen() const;
	void Show(WindowShowCommand command = WindowShowCommand::Show);
	auto GetSysData() -> const WindowData&;
	auto Position() const -> math::ivec2;
	void Move(const math::ivec2& pos);
	auto Size() const -> math::ivec2;
	void Resize(const math::ivec2& size);
	auto SurfaceSize() const -> math::ivec2;
	void ResizeSurface(const math::ivec2& size);
	void SetTitle(const char8_t* title);
	void ChangeFrameStyle(WindowFrameStyle style);
	bool Minimizable() const;
	void MakeMinimizable(bool state);
	auto GetLoop() const -> SystemLoop&;
	using PaintHandler = void(WndEvtDt);
	auto SubscribePaintEvent(std::function<PaintHandler>&& c)
	-> notifier::connection<PaintHandler>;
	bool Register(WindowEvent evt, void* object, void(*cb)());
	void Unregister(WindowEvent evt, void* object, void(*cb)()) noexcept;
	template <WindowEvent evt>
	bool Register(const util::function_ptr<typename util::event_traits<evt>::handler>& cb);
	template <WindowEvent evt>
	void Unregister(const util::function_ptr<typename util::event_traits<evt>::handler>& cb) noexcept;
private:
    util::impl_ptr<Window_impl> impl;
};

template <WindowEvent evt>
bool Window::Register(const util::function_ptr<typename util::event_traits<evt>::handler>& cb)
{
    return Register(evt, cb.get_object_ptr(), reinterpret_cast<void(*)()>(cb.get_function()));
}

template <WindowEvent evt>
void Window::Unregister(const util::function_ptr<typename util::event_traits<evt>::handler>& cb) noexcept
{
    Unregister(evt, cb.get_object_ptr(), reinterpret_cast<void(*)()>(cb.get_function()));
}

} /* namespace dse::core */

#endif /* DSE_CORE_WINDOW_H_ */
