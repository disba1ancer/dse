/*
 * Window_win32.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef WINDOW_WIN32_H_
#define WINDOW_WIN32_H_

#include "dse/core/SystemLoop.h"
#include <dse/core/win32.h>
#include <swal/window.h>
#include <dse/core/Window.h>
#include <dse/notifier/notifier.h>

namespace dse::core {

class Window_win32 {
public:
	Window_win32(SystemLoop& loop);
	~Window_win32();
	Window_win32(const Window_win32 &other) = delete;
	Window_win32(Window_win32 &&other) = delete;
	Window_win32& operator=(const Window_win32 &other) = delete;
	Window_win32& operator=(Window_win32 &&other) = delete;
	bool IsVisible() const;
	void Show(WindowShowCommand command = WindowShowCommand::Show);
	auto GetSysData() -> const WindowData&;
	auto Size() const -> math::ivec2;
	void Resize(const math::ivec2& size);
	void SetTitle(const char8_t* title);
	void ChangeFrameStyle(WindowFrameStyle style);
	bool HasMinimizeCtl();
	void ShowMinimizeCtl(bool state);
	bool HasMaximizeCtl();
	void ShowMaximizeCtl(bool state);
	auto GetLoop() const -> SystemLoop&;
	auto SubscribeCloseEvent(std::function<Window::CloseHandler>&& c)
	-> notifier::connection<Window::CloseHandler>;
	auto SubscribeResizeEvent(std::function<Window::ResizeHandler>&& c)
	-> notifier::connection<Window::ResizeHandler>;
	auto SubscribeKeyEvent(std::function<Window::KeyHandler>&& c)
	-> notifier::connection<Window::KeyHandler>;
	auto SubscribePaintEvent(std::function<Window::PaintHandler>&& c)
	-> notifier::connection<Window::PaintHandler>;
	auto SubscribeMouseMoveEvent(std::function<Window::MouseMoveHandler>&& c)
	-> notifier::connection<Window::MouseMoveHandler>;
private:
    enum Constants {
        GwlpThis = 0
    };

    enum class WindowFlags {
        Minimizable = 1,
        Maximizable = 2
    };
    friend void enable(util::enum_bit_ops<WindowFlags>);

	auto WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT;
	static auto WindowClass() -> LPCTSTR;
	static auto DefMinSize() -> math::ivec2;
	static auto DefMaxSize() -> math::ivec2;
	auto MinSize() const -> math::ivec2;
	auto MaxSize() const -> math::ivec2;
	auto ClampSize(const math::ivec2& size) const -> math::ivec2;
	struct WinStyles
	{
		DWORD exStyle;
		DWORD style;
	};
	auto CalcStyles() -> WinStyles;
	void ApplyStyles();

	auto OnPaint(WindowEventData_win32& d) -> LRESULT;
	auto OnClose(WindowEventData_win32& d) -> LRESULT;
	auto OnSize(WindowEventData_win32& d) -> LRESULT;
	auto OnKeyDown(WindowEventData_win32& d) -> LRESULT;
	auto OnKeyUp(WindowEventData_win32& d) -> LRESULT;
	auto OnMouseMove(WindowEventData_win32& d) -> LRESULT;
	auto OnPosChanging(WindowEventData_win32& d) -> LRESULT;
	auto OnPosChanged(WindowEventData_win32& d) -> LRESULT;
	auto OnNCCreate(WindowEventData_win32& d) -> LRESULT;
	auto OnGetMinMaxInfo(WindowEventData_win32& d) -> LRESULT;
	auto CallDefWindowProc(WindowEventData_win32& d) -> LRESULT;

	notifier::notifier<Window::CloseHandler> closeSubscribers;
	notifier::notifier<Window::ResizeHandler> resizeSubscribers;
	notifier::notifier<Window::KeyHandler> keySubscribers;
	notifier::notifier<Window::PaintHandler> paintSubscribers;
	notifier::notifier<Window::MouseMoveHandler> mouseMoveSubscribers;
	math::ivec2 pos;
	math::ivec2 size;
	math::ivec2 minSize = {-1, 0};
	math::ivec2 maxSize = {-1, 0};
	WindowFrameStyle frameStyle = WindowFrameStyle::Sizable;
	WindowFlags flags = WindowFlags::Minimizable | WindowFlags::Maximizable;
	SystemLoop* loop;
	swal::Window wnd;
};

} /* namespace dse::core */

#endif /* WINDOW_WIN32_H_ */
