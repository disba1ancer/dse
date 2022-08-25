/*
 * Window.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_WINDOW_H_
#define DSE_CORE_WINDOW_H_

#include <memory>
#include "WindowShowCommand.h"
#include <dse/notifier/notifier.h>
#include "KeyboardKeyState.h"
#include <dse/math/vec.h>

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

class Window {
	std::unique_ptr<Window_impl> impl;
public:
	Window();
	~Window();
	Window(const Window &other) = delete;
	Window(Window &&other) = delete;
	Window& operator=(const Window &other) = delete;
	Window& operator=(Window &&other) = delete;
	bool IsVisible() const;
	void Show(WindowShowCommand command = WindowShowCommand::Show);
	const WindowData& GetSysData();
	math::ivec2 Size();
	void Resize(const math::ivec2& size);
	typedef void(SimpleHandler)(WndEvtDt);
	typedef SimpleHandler CloseHandler;
	auto SubscribeCloseEvent(std::function<CloseHandler>&& c) -> notifier::connection<CloseHandler>;
	typedef void(ResizeHandler)(WndEvtDt, int, int, WindowShowCommand);
	auto SubscribeResizeEvent(std::function<ResizeHandler>&& c) -> notifier::connection<ResizeHandler>;
	typedef void(KeyHandler)(WndEvtDt, KeyboardKeyState, int);
	auto SubscribeKeyEvent(std::function<KeyHandler>&& c) -> notifier::connection<KeyHandler>;
	typedef void(PaintHandler)(WndEvtDt);
	auto SubscribePaintEvent(std::function<PaintHandler>&& c) -> notifier::connection<PaintHandler>;
	typedef void(MouseMoveHandler)(WndEvtDt, int x, int y);
	auto SubscribeMouseMoveEvent(std::function<MouseMoveHandler>&& c) -> notifier::connection<MouseMoveHandler>;
};

} /* namespace dse::core */

#endif /* DSE_CORE_WINDOW_H_ */
