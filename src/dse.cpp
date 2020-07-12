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
#include <cstdint>
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
#include "scn/Material.h"
#include <functional>

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
using dse::threadutils::TaskState;
using dse::scn::Material;

ExecutionThread mainThread;

constexpr auto test = dse::math::norm(dse::math::vec3{1, 1, 1});

int main(int , char* []) {
	Window window;
	RenderOpenGL31 render(window);

	setMouseCursorPosWndRel(window.size() / 2, window);
	window.show();
	Camera cam;
	Scene scene;
	Cube cubeMesh;
	Material mat(dse::math::vec4{.2f, .8f, .4f, 1.f});
	auto cube1 = scene.createObject(Object(&cubeMesh));
	auto cube2 = scene.createObject(Object(&cubeMesh));
	cube1->setPos({-1.f, -1.f, -1.f});
	//cube1->setScale({.25f, .25f, .25f});
	cube1->setMaterial(0, &mat);
	cube2->setPos({1.f, 1.f, 1.f});
	//cube2->setScale({.25f, .25f, .25f});
	cube2->setMaterial(0, &mat);
	render.setScene(scene);
	cam.setPos({0.f, -4.f, 0.f});
	cam.setRot({std::sqrt(2.f) * 0.5, 0.f, 0.f, std::sqrt(2.f) * 0.5});
	cam.setNear(.03125f);
	cam.setFar(1024.f);
	render.setCamera(cam);
	auto closeCon = window.subscribeCloseEvent([](WndEvtDt){
		mainThread.exit(0);
	});
	float pitch = dse::math::PI * 0.5, yaw = 0.f;
	float spd = 0.f, sdspd = 0.f;
	auto keyCon = window.subscribeKeyEvent([&spd, &sdspd](WndEvtDt, KeyboardKeyState cmd, int key){
		constexpr auto speed = .015625f;
		switch (key) {
		case 'W':
			if (cmd == KeyboardKeyState::DOWN) spd += speed;
			if (cmd == KeyboardKeyState::UP) spd -= speed;
			break;
		case 'A':
			if (cmd == KeyboardKeyState::DOWN) sdspd -= speed;
			if (cmd == KeyboardKeyState::UP) sdspd += speed;
			break;
		case 'S':
			if (cmd == KeyboardKeyState::DOWN) spd -= speed;
			if (cmd == KeyboardKeyState::UP) spd += speed;
			break;
		case 'D':
			if (cmd == KeyboardKeyState::DOWN) sdspd += speed;
			if (cmd == KeyboardKeyState::UP) sdspd -= speed;
			break;
		case 27:
			mainThread.exit(0);
			break;
//		default:
//			std::printf("%i %i\n", static_cast<int>(cmd), key);
//			std::fflush(stdout);
		}
	});
	auto mmoveCon = window.subscribeMouseMoveEvent([&window, &pitch, &yaw, &cam](WndEvtDt, int x, int y) {
		using namespace dse::math;
		auto center = window.size() / 2;
		auto moffset = dse::math::ivec2{x, y} - center;
		constexpr float sens = 3.f / 4096.f;
		pitch = std::clamp(pitch - moffset[1] * sens, 0.f, PI);
		yaw -= moffset[0] * sens;
		auto pitchHalf = pitch * .5f;
		auto yawHalf = yaw * .5f;
		auto camrot = qmul(vec4{ 0, 0, std::sin(yawHalf), std::cos(yawHalf) }, { std::sin(pitchHalf), 0, 0, std::cos(pitchHalf) });
		cam.setRot(camrot);
		setMouseCursorPosWndRel(center, window);
	});
	auto sizeCon = window.subscribeResizeEvent([](WndEvtDt, int, int, WindowShowCommand) {
		mainThread.yieldTasks();
	});
	mainThread.addTask(dse::os::nonLockLoop());
	mainThread.addTask(make_handler<&RenderOpenGL31::renderTask>(render));
	mainThread.addTask([&cube1, &cube2, &cam, &spd, &sdspd] {
		using namespace dse::math;
		auto angle = 1.f;
		auto axe1 = norm(-vec3{1.f, 1.f, 1.f}) * std::sin(PI * angle / 360.f);
		auto cs1 = std::cos(PI * angle / 360.f);
		auto rslt = norm(qmul({axe1[X], axe1[Y], axe1[Z], cs1}, cube1->getQRot()));
		cube1->setQRot(rslt);
		//cube2->setQRot(rslt);
		if (spd != 0.f || sdspd != 0.f) cam.setPos(cam.getPos() + vecrotquat(norm(vec3{0.f, 0.f, -1.f} * spd + vec3{1.f, 0.f, 0.f} * sdspd), cam.getRot()) );
		return TaskState::YIELD;
	});
	return mainThread.runOnCurent();
}
