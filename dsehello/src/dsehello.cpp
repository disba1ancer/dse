//============================================================================
// Name        : dsehello.cpp
// Author      : disba1ancer
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <dse/util.h>
#include <iostream>

using namespace std;
using namespace dse::util;

class A;

struct B : OwnerStore<A, B> {
	int field;
public:
	B(A* owner) : OwnerStore<A, B>(owner), field() {}
};

struct A {
	int field;
	B b = this;
};

int main() {
	A a;
	B* b = &a.b;
	b->getOwner()->field = 21;
	cout << a.field << endl;
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
