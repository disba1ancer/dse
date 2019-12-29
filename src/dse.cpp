/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "threadutils/ExecutionThread.h"
#include "os/loop.h"
#include "os/Window.h"
#include <cstdio>

using dse::threadutils::ExecutionThread;
using dse::os::Window;
using dse::os::WindowShowCommand;
using dse::os::KeyboardKeyState;

ExecutionThread mainThread;

int main(int argc, char* argv[]) {
	Window window;
	window.show();
	auto con1 = window.subscribeCloseEvent([&window, &argc, &argv](){
		mainThread.exit(0);
	});
	auto con2 = window.subscribeResizeEvent([&window, &argc, &argv](WindowShowCommand cmd, int width, int height){
		std::printf("%i %i %i\n", static_cast<int>(cmd), width, height);
		std::fflush(stdout);
	});
	auto con3 = window.subscribeKeyEvent([&window, &argc, &argv](KeyboardKeyState cmd, int key, int scancode){
		std::printf("%i %i %i\n", static_cast<int>(cmd), key, scancode);
		std::fflush(stdout);
	});
	mainThread.addTask(dse::os::nonLockLoop);
	return mainThread.runOnCurent();
}
