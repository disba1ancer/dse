/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 ${user}.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * Terminal.h
 *
 *  Created on: 23 авг. 2017 г.
 *      Author: disba1ancer
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "obscon.h"
#include "Event.h"
#include "util.h"

namespace dse {
namespace core {

class Terminal {
public:
	enum EventType {
		EVENT_ON_CLOSE,
		EVENT_ON_RESIZE,
		EVENT_ON_KEY_INPUT,
		EVENT_ON_REDRAW,
		EVENT_COUNT
	};

	enum ShowCommand {
		HIDE,
		SHOW,
		SHOW_MINIMIZED,
		SHOW_RESTORED,
		SHOW_MAXIMIZED,
		SHOW_FULL_SCREEN
	};

	Terminal();
	Terminal(const Terminal& orig) = delete;
	Terminal(Terminal&& orig) = delete;
	Terminal& operator =(const Terminal& orig) = delete;
	Terminal& operator =(Terminal&& orig) = delete;
	~Terminal();
	const void* getSysDepId() const;
	bool isVisible() const;
	void show(ShowCommand command = SHOW);
	int getHeight() const;
	int getWidth() const;
	void resize(int width, int height);
	bool isSizable() const;
	void makeSizable(bool sizable = true);
	Connection<void> attach(EventType eventType, void* owner, void (*callback)(void*, Event*));
	/*template <typename Recv, typename Evt>
	Connection<void> attach(EventType eventType, Recv* recv, void (Recv::*method)(Evt&));*/
	void close();
	void forceRedraw();

private:
	//unsigned char storage[256 - sizeof(void*)];
	void* priv;
};

/*template <typename Recv, typename Evt>
Connection<void> Terminal::attach(EventType eventType, Recv* recv, void (Recv::*method)(Evt&)) {
	void (*wrapped)(Recv*, Evt*);
	wrapped = (&::dse::util::typename wrap_method<decltype(method)>::template wrapper<method>);
	static_cast<void>(wrapped);
	return Connection<void>(nullptr, nullptr);//attach(eventType, recv, &::dse::util::wrap_static<decltype(wrapped)>::template wrapper<wrapped, void, void*, Event*>);//DSE_WRAP_STATIC(DSE_WRAP_METHOD(method), void, void*, Event*)
}*/

} /* namespace core */
} /* namespace dse */

#endif /* TERMINAL_H_ */
