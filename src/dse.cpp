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
#include "CustomPainter.h"

using dse::threadutils::ExecutionThread;
using dse::os::Window;
using dse::os::WindowShowCommand;
using dse::os::KeyboardKeyState;
using dse::os::WndEvtDt;
using dse::os::PntEvtDt;

ExecutionThread mainThread;

int main(int argc, char* argv[]) {
	Window window;
	CustomPainter painter(window);
	window.show();
	auto con1 = window.subscribeCloseEvent([](WndEvtDt){
		mainThread.exit(0);
	});
	auto con2 = window.subscribeResizeEvent([](WndEvtDt, int width, int height, WindowShowCommand cmd){
		std::printf("%i %i %i\n", static_cast<int>(cmd), width, height);
		std::fflush(stdout);
	});
	auto con3 = window.subscribeKeyEvent([](WndEvtDt, KeyboardKeyState cmd, int key){
		std::printf("%i %i\n", static_cast<int>(cmd), key);
		std::fflush(stdout);
	});
	mainThread.addTask(dse::os::nonLockLoop);
	mainThread.addTask([&painter](){
		painter.invalidate();
		return true;
	});
	return mainThread.runOnCurent();
}
