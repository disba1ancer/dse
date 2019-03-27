/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 disba1ancer.
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
//============================================================================
// Name        : dsehello.cpp
// Author      : disba1ancer
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <dse/util.h>
#include <dse/Terminal.h>
#include <dse/CloseEvent.h>
#include <dse/RedrawEvent.h>
#include <iostream>
#include <string>

using namespace std;
using namespace dse::util;
using namespace dse::core;

class Subscriber {
	Terminal term;
	void onClose(CloseEvent&) {
		returnMainLoop(0);
	}
	void onRedraw(RedrawEvent&) {
		cout << "redraw1" << endl;
	}
	Connection<Subscriber> closeCon = term.attach(Terminal::EVENT_ON_CLOSE, this, DSE_HANDLER(&onClose));
	Connection<Subscriber> redrawCon = term.attach(Terminal::EVENT_ON_REDRAW, this, DSE_HANDLER(&onRedraw));
public:
	Subscriber()
	{
		term.show();
	}

	void step() {
		term.forceRedraw();
	}
};

int main(int , char* []) {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	Subscriber s;
	//auto lam = [&s]{ s.step(); };
	return mainLoop([&s]{ s.step(); });
}
