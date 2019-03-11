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
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace std;
using namespace dse::util;
using namespace dse::core;

/*class A;

struct B : OwnerStore<A, B> {
	int field;
public:
	B(A* owner) : OwnerStore<A, B>(owner), field() {}
};

struct A {
	int field;
	B b = this;
};*/

class Subscriber {
	Terminal term;
	Connection<Subscriber> closeCon;
	void onClose(CloseEvent&) {
		PostQuitMessage(0);
	}
public:
	Subscriber() :
		closeCon(term.attach(Terminal::EVENT_ON_CLOSE, this, DSE_HANDLER(&onClose)))
	{
		term.show();
	}
};

int main(int , char* []) {
	/*A a;
	B* b = &a.b;
	b->getOwner()->field = 21;
	cout << a.field << endl;*/
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	/*Terminal window;
	Connection<void> closeCon = window.attach(Terminal::EVENT_ON_CLOSE, nullptr, [](void*, Event*){
		PostQuitMessage(0);
	});
	window.show();
	dse::util::reference_to_pointer<int&>::type test = 0;
	int* tst2 = test;
	static_cast<void>(tst2);*/
	Subscriber s;
	MSG msg;
	BOOL retcode;
	while ((retcode = GetMessage(&msg, 0, 0, 0))) {
		if (retcode == -1) {
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}
