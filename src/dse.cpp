/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "threadutils/ExecutionThread.h"
#include "os/loop.h"
#include "os/Window.h"

using dse::threadutils::ExecutionThread;
using dse::os::Window;

ExecutionThread mainThread;

int main(int argc, char* argv[]) {
	std::unique_ptr<Window> window(new Window);
	window->show();
	mainThread.addTask(dse::os::nonLockLoop);
	return mainThread.runOnCurent();
}
