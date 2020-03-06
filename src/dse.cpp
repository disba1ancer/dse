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
#include "subsys/RenderOpenGL31.h"
#include "notifier/make_handler.h"
#include "scn/Scene.h"
#include "scn/Cube.h"
#include "scn/Object.h"
#include <cmath>
#include "math/qmath.h"

using dse::threadutils::ExecutionThread;
using dse::os::Window;
using dse::os::WindowShowCommand;
using dse::os::KeyboardKeyState;
using dse::os::WndEvtDt;
using dse::os::PntEvtDt;
using dse::subsys::RenderOpenGL31;
using dse::notifier::make_handler;
using dse::scn::Cube;
using dse::scn::Scene;
using dse::scn::Object;

ExecutionThread mainThread;

int main(int , char* []) {
	Window window;
	RenderOpenGL31 render(window);

	window.show();
	Scene scene;
	Cube cubeMesh;
	auto cube1 = scene.createObject(Object(&cubeMesh));
	auto cube2 = scene.createObject(Object(&cubeMesh));
	cube1->setPos({-.25f, -.25f, -.25f});
	cube1->setScale({.25f, .25f, .25f});
	cube2->setPos({.25f, .25f, .25f});
	cube2->setScale({.25f, .25f, .25f});
	render.setScene(scene);
	auto closeCon = window.subscribeCloseEvent([](WndEvtDt){
		mainThread.exit(0);
	});
	auto keyCon = window.subscribeKeyEvent([](WndEvtDt, KeyboardKeyState cmd, int key){
		std::printf("%i %i\n", static_cast<int>(cmd), key);
		std::fflush(stdout);
	});
	mainThread.addTask(dse::os::nonLockLoop());
	mainThread.addTask(make_handler<&RenderOpenGL31::renderTask>(render));
	mainThread.addTask([&cube1, &cube2]{
		constexpr float PI = 3.14159265f;
		using namespace dse::math;
		auto axe = norm(vec3{1.f, 1.f, 1.f}) * std::sin(PI * .125f / 360.f);
		auto cs = std::cos(PI * .0125f / 360.f);
		auto rslt = qmul(norm(cube1->getQRot()), {axe[X], axe[Y], axe[Z], cs});
		cube1->setQRot(rslt);
		cube2->setQRot(rslt);
		return true;
	});
	return mainThread.runOnCurent();
}
