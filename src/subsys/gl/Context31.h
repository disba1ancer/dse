/*
 * Context.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_CONTEXT_H_
#define SUBSYS_GL_CONTEXT_H_

#ifdef _WIN32
#include "../../os/win32.h"
#endif
#include "../../os/Window.h"

namespace dse {
namespace subsys {
namespace gl {

class Context31 {
#ifdef _WIN32
	HWND hWnd;
	HDC hdc;
	HGLRC glrc;
#endif
public:
	Context31(os::Window& wnd);
	~Context31();
	Context31(const Context31 &other) = delete;
	Context31(Context31 &&other) = delete;
	Context31& operator=(const Context31 &other) = delete;
	Context31& operator=(Context31 &&other) = delete;
	void SwapBuffers();
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_CONTEXT_H_ */
