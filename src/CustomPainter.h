/*
 * CustomPainter.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef CUSTOMPAINTER_H_
#define CUSTOMPAINTER_H_

#include "os/Window.h"

class CustomPainter {
	dse::os::Window* wnd;
	dse::notifier::connection<dse::os::Window::PaintHandler> paintCon;
public:
	CustomPainter(dse::os::Window& wnd);
	CustomPainter(const CustomPainter&) = delete;
	CustomPainter(CustomPainter&&) = delete;
	CustomPainter& operator=(const CustomPainter&) = delete;
	CustomPainter& operator=(CustomPainter&&) = delete;
	void paint(dse::os::WndEvtDt);
	void invalidate();
};

#endif /* CUSTOMPAINTER_H_ */
