/*
 * Window.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_WINDOW_H_
#define OS_WINDOW_H_

#include <memory>
#include "WindowShowCommand.h"
#include "../notifier/notifier.h"
#include "KeyboardKeyState.h"
#include "../math/vec.h"

namespace dse {
namespace os {

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
	bool isVisible() const;
	void show(WindowShowCommand command = WindowShowCommand::SHOW);
	const WindowData& getSysData();
	math::ivec2 size();
	void resize(const math::ivec2& size);
	typedef void(SimpleHandler)(WndEvtDt);
	typedef SimpleHandler CloseHandler;
	notifier::connection<CloseHandler> subscribeCloseEvent(std::function<CloseHandler>&& c);
	typedef void(ResizeHandler)(WndEvtDt, int, int, WindowShowCommand);
	notifier::connection<ResizeHandler> subscribeResizeEvent(std::function<ResizeHandler>&& c);
	typedef void(KeyHandler)(WndEvtDt, KeyboardKeyState, int);
	notifier::connection<KeyHandler> subscribeKeyEvent(std::function<KeyHandler>&& c);
	typedef void(PaintHandler)(WndEvtDt, PntEvtDt);
	notifier::connection<PaintHandler> subscribePaintEvent(std::function<PaintHandler>&& c);
	typedef void(MouseMoveHandler)(WndEvtDt, int x, int y);
	notifier::connection<MouseMoveHandler> subscribeMouseMoveEvent(std::function<MouseMoveHandler>&& c);
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_WINDOW_H_ */
