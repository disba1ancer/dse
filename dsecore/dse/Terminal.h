/*
 * Terminal.h
 *
 *  Created on: 23 авг. 2017 г.
 *      Author: Anton
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "obscon.h"
#include "Event.h"

namespace dse {
namespace core {

class Terminal {
public:
	enum EventType {
		EVENT_ON_CLOSE,
		EVENT_ON_RESIZE,
		EVENT_ON_KEY_INPUT,
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
	void attach(EventType eventType, void* owner, void (*)(void*, Event*));
	void close();

private:
	//unsigned char storage[256 - sizeof(void*)];
	void* priv;
};

} /* namespace core */
} /* namespace dse */

#endif /* TERMINAL_H_ */
