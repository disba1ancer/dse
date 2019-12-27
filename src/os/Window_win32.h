/*
 * Window_win32.h
 *
 *  Created on: 27 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef OS_WINDOW_WIN32_H_
#define OS_WINDOW_WIN32_H_

#include <windows.h>
#include <map>
#include "WindowShowCommand.h"

namespace dse {
namespace os {

class Window_win32 {
	HWND hWnd;
	static std::map<HWND, Window_win32*> hWndMap;
private:
	static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static ATOM makeWindowClsID();
public:
	Window_win32();
	~Window_win32();
	Window_win32(const Window_win32 &other) = delete;
	Window_win32(Window_win32 &&other) = delete;
	Window_win32& operator=(const Window_win32 &other) = delete;
	Window_win32& operator=(Window_win32 &&other) = delete;
	bool isVisible() const;
	void show(WindowShowCommand command = WindowShowCommand::SHOW);
};

} /* namespace os */
} /* namespace dse */

#endif /* OS_WINDOW_WIN32_H_ */
