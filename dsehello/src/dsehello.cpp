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
