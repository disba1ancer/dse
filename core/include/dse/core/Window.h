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
#include <dse/util/handle.h>

namespace dse::core {

#ifdef _WIN32

class Window_win32;
struct WindowData_win32;
struct WindowEventData_win32;
struct PaintEventData_win32;

using Window_impl = Window_win32;
using WindowData = WindowData_win32;

#endif

enum class WindowEventHandle : std::size_t {};

class API_DSE_CORE Window;

} // namespace dse::core

template <>
struct dse::util::handle_traits<dse::core::WindowEventHandle> {
    using sender = dse::core::Window;
    static void kill_handle(sender&, dse::core::WindowEventHandle);
};

namespace dse::core {

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
    auto GetSysData() -> WindowData;
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
    // using PaintHandler = void(WndEvtDt);
    auto Register(WindowEvent evt, void* object, void(*cb)()) -> std::size_t;
    void Unregister(std::size_t id) noexcept;
    template <WindowEvent evt>
    auto Register(const util::function_ptr<typename util::event_traits<evt>::handler>& cb) -> util::handle_owner<WindowEventHandle>;
private:
    util::impl_ptr<Window_impl> impl;
};

template <WindowEvent evt>
auto Window::Register(const util::function_ptr<typename util::event_traits<evt>::handler>& cb) -> util::handle_owner<WindowEventHandle>
{
    return {*this, WindowEventHandle(Register(evt, cb.get_object_ptr(), reinterpret_cast<void(*)()>(cb.get_function())))};
}

} // namespace dse::core

inline void dse::util::handle_traits<dse::core::WindowEventHandle>::kill_handle(sender& s, dse::core::WindowEventHandle h)
{
    s.Unregister(std::to_underlying(h));
}

#endif /* DSE_CORE_WINDOW_H_ */
