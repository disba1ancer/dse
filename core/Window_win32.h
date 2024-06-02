/*
 * Window_win32.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef WINDOW_WIN32_H_
#define WINDOW_WIN32_H_

#include "dse/core/UILoop.h"
#include <dse/core/win32.h>
#include <swal/window.h>
#include <map>
#include <dse/core/Window.h>
#include <dse/notifier/notifier.h>

namespace dse::core {

class Window_win32 {
	notifier::notifier<Window::CloseHandler> closeSubscribers;
	notifier::notifier<Window::ResizeHandler> resizeSubscribers;
	notifier::notifier<Window::KeyHandler> keySubscribers;
	notifier::notifier<Window::PaintHandler> paintSubscribers;
	notifier::notifier<Window::MouseMoveHandler> mouseMoveSubscribers;
	swal::Window wnd;

    enum Constants {
        GwlpThis = 0
    };

	//static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
	static ATOM makeWindowClsID();
	LRESULT onPaint(WindowEventData_win32& d);
	LRESULT onClose(WindowEventData_win32& d);
	LRESULT onSize(WindowEventData_win32& d);
	LRESULT onKeyDown(WindowEventData_win32& d);
	LRESULT onKeyUp(WindowEventData_win32& d);
	LRESULT onMouseMove(WindowEventData_win32& d);
    static auto MakeWindow(UILoop& loop, Window_win32* window) -> swal::Window;
public:
	Window_win32(UILoop& loop);
	~Window_win32();
	Window_win32(const Window_win32 &other) = delete;
	Window_win32(Window_win32 &&other) = delete;
	Window_win32& operator=(const Window_win32 &other) = delete;
	Window_win32& operator=(Window_win32 &&other) = delete;
	bool isVisible() const;
	void show(WindowShowCommand command = WindowShowCommand::Show);
	const WindowData& getSysData();
	math::ivec2 size();
	void resize(const math::ivec2& size);
	notifier::connection<Window::CloseHandler> subscribeCloseEvent(std::function<Window::CloseHandler>&& c);
	notifier::connection<Window::ResizeHandler> subscribeResizeEvent(std::function<Window::ResizeHandler>&& c);
	notifier::connection<Window::KeyHandler> subscribeKeyEvent(std::function<Window::KeyHandler>&& c);
	notifier::connection<Window::PaintHandler> subscribePaintEvent(std::function<Window::PaintHandler>&& c);
	notifier::connection<Window::MouseMoveHandler> subscribeMouseMoveEvent(std::function<Window::MouseMoveHandler>&& c);
};

} /* namespace dse::core */

#endif /* WINDOW_WIN32_H_ */
