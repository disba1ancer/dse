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
#include "math/constants.h"
#include "scn/Camera.h"
#include "os/mcursor.h"
#include <algorithm>

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
using dse::scn::Camera;
using dse::os::setMouseCursorPosWndRel;

ExecutionThread mainThread;

constexpr auto test = dse::math::norm(dse::math::vec3{1, 1, 1});

int main(int , char* []) {
	Window window;
	RenderOpenGL31 render(window);

	window.show();
	Camera cam;
	Scene scene;
	Cube cubeMesh;
	auto cube1 = scene.createObject(Object(&cubeMesh));
	auto cube2 = scene.createObject(Object(&cubeMesh));
	cube1->setPos({-1.f, -1.f, -1.f});
	//cube1->setScale({.25f, .25f, .25f});
	cube2->setPos({1.f, 1.f, 1.f});
	//cube2->setScale({.25f, .25f, .25f});
	render.setScene(scene);
	cam.setPos({0.f, -4.f, 0.f});
	cam.setRot({std::sqrt(2.f) * 0.5, 0.f, 0.f, std::sqrt(2.f) * 0.5});
	/*cam.setNear(1.24f);
	cam.setFar(16.f);*/
	render.setCamera(cam);
	auto closeCon = window.subscribeCloseEvent([](WndEvtDt){
		mainThread.exit(0);
	});
	float pitch = 90.f, yaw = 0.f;
	float spd = 0.f, sdspd = 0.f;
	auto keyCon = window.subscribeKeyEvent([&spd, &sdspd](WndEvtDt, KeyboardKeyState cmd, int key){
		switch (key) {
		case 'W':
			if (cmd == KeyboardKeyState::DOWN) spd += .125f;
			if (cmd == KeyboardKeyState::UP) spd -= .125f;
			break;
		case 'A':
			if (cmd == KeyboardKeyState::DOWN) sdspd -= .125f;
			if (cmd == KeyboardKeyState::UP) sdspd += .125f;
			break;
		case 'S':
			if (cmd == KeyboardKeyState::DOWN) spd -= .125f;
			if (cmd == KeyboardKeyState::UP) spd += .125f;
			break;
		case 'D':
			if (cmd == KeyboardKeyState::DOWN) sdspd += .125f;
			if (cmd == KeyboardKeyState::UP) sdspd -= .125f;
			break;
		case 27:
			mainThread.exit(0);
			break;
		default:
			std::printf("%i %i\n", static_cast<int>(cmd), key);
			std::fflush(stdout);
		}
	});
	auto mmoveCon = window.subscribeMouseMoveEvent([&window, &pitch, &yaw](WndEvtDt, int x, int y) {
		auto center = window.size() / 2;
		auto moffset = dse::math::ivec2{x, y} - center;
		std::printf("%i %i\n", moffset[0], moffset[1]);
		std::fflush(stdout);
		pitch = std::clamp(pitch - moffset[1] * .0625f, 0.f, 180.f);
		yaw -= moffset[0] * 0.0625;
		setMouseCursorPosWndRel(center, window);
	});
	mainThread.addTask(dse::os::nonLockLoop());
	mainThread.addTask(make_handler<&RenderOpenGL31::renderTask>(render));
	mainThread.addTask([&cube1, &cube2, &cam, &spd, &sdspd, &pitch, &yaw] {
		using namespace dse::math;
		auto axe1 = norm(-vec3{1.f, 1.f, 1.f}) * std::sin(PI * .25f / 360.f);
		auto cs1 = std::cos(PI * .0125f / 360.f);
		auto rslt = norm(qmul({axe1[X], axe1[Y], axe1[Z], cs1}, cube1->getQRot()));
		cube1->setQRot(rslt);
		//cube2->setQRot(rslt);
		auto pitchRad = (pitch) * PI / 360.f;
		auto yawRad = (yaw) * PI / 360.f;
		auto camrot = qmul(vec4{ 0, 0, std::sin(yawRad), std::cos(yawRad) }, { std::sin(pitchRad), 0, 0, std::cos(pitchRad) });
		cam.setRot(camrot);
		if (spd != 0.f || sdspd != 0.f) cam.setPos(cam.getPos() + vecrotquat(vec3{0.f, 0.f, -1.f} * spd + vec3{1.f, 0.f, 0.f} * sdspd, camrot) );
		return true;
	});
	return mainThread.runOnCurent();
}
