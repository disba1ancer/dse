/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "util/ExecutionThread.h"
#include "os/loop.h"
#include "os/Window.h"
#include <cstdio>
#include <cstdint>
#include "renders/RenderOpenGL31.h"
#include "util/functional.h"
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
#include <algorithm>
#include "core/ThreadPool.h"
#include "core/File.h"
#include "util/scope_exit.h"

#include <swal/error.h>
#include <string>

using namespace std::string_literals;
using dse::util::ExecutionThread;
using dse::os::Window;
using dse::os::WindowShowCommand;
using dse::os::KeyboardKeyState;
using dse::os::WndEvtDt;
using dse::os::PntEvtDt;
using dse::renders::RenderOpenGL31;
using dse::util::from_method;
using dse::scn::Cube;
using dse::scn::Scene;
using dse::scn::Object;
using dse::scn::Camera;
using dse::os::setMouseCursorPosWndRel;
using dse::util::TaskState;
using dse::scn::Material;
using dse::core::ThreadPool;
using dse::core::PoolCaps;
using dse::core::File;
using dse::core::OpenMode;

//ExecutionThread mainThread;
ThreadPool thrPool;

int main(int argc, char* argv[]) {
	Window window;
	RenderOpenGL31 render(window);
	window.show();
	setMouseCursorPosWndRel(window.size() / 2, window);
	auto root = [&window, &render, argc, argv]([[maybe_unused]] ThreadPool& pool) -> dse::core::pool_task {
		File file(thrPool, u8"test.txt", OpenMode::READ);
		std::byte buf[4096];
		if (file.isValid()) {
			auto r = co_await file.readAsync(buf, 4096);
			std::printf("%.*s\n", int(r.transfered), reinterpret_cast<char*>(buf));
		} else {
			std::printf("%s", file.status().message().data());
		}
		std::fflush(stdout);

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
		cam.setRot({std::sqrt(2.f) * 0.5f, 0.f, 0.f, std::sqrt(2.f) * 0.5f});
		cam.setNear(.03125f);
		cam.setFar(1024.f);
		render.setCamera(cam);
		auto closeCon = window.subscribeCloseEvent([](WndEvtDt){
			thrPool.stop();
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
				thrPool.stop();
				break;
			default:
				std::printf("%i %i\n", static_cast<int>(cmd), key);
				std::fflush(stdout);
			}
		});
		auto mmoveCon = window.subscribeMouseMoveEvent([&window, &pitch, &yaw, &cam](WndEvtDt, int x, int y) {
			using namespace dse::math;
			auto center = window.size() / 2;
			auto moffset = dse::math::ivec2{x, y} - center;
			constexpr float sens = 3.f / 4096.f;
			pitch = std::clamp(pitch - moffset.y() * sens, 0.f, PI);
			yaw -= moffset.x() * sens;
			auto pitchHalf = pitch * .5f;
			auto yawHalf = yaw * .5f;
			auto camrot = qmul(vec4{ 0, 0, std::sin(yawHalf), std::cos(yawHalf) }, vec4{ std::sin(pitchHalf), 0, 0, std::cos(pitchHalf) });
			cam.setRot(camrot);
			setMouseCursorPosWndRel(center, window);
		});
		/*auto sizeCon = window.subscribeResizeEvent([](WndEvtDt, int, int, WindowShowCommand) {
			mainThread.yieldTasks();
		});
		mainThread.addTask(dse::os::nonLockLoop());*/
		while (true) {
			using namespace dse::math;
			auto angle = 1.f;
			auto axe1 = norm(-vec3{1.f, 1.f, 1.f}) * std::sin(PI * angle / 360.f);
			auto cs1 = std::cos(PI * angle / 360.f);
			auto rslt = norm(qmul(vec4{axe1.x(), axe1.y(), axe1.z(), cs1}, cube1->getQRot()));
			cube1->setQRot(rslt);
			//cube2->setQRot(rslt);
			if (spd != 0.f || sdspd != 0.f) {
				auto movement = norm(vec3{0.f, 0.f, -1.f} * spd + vec3{1.f, 0.f, 0.f} * sdspd);
				cam.setPos(cam.getPos() + vecrotquat(movement, cam.getRot()));
			}
			//renderTask.reset(make_handler<&RenderOpenGL31::renderTask>(render));
			//renderTask.then([&stepTask]{ thrPool.schedule(stepTask); });
			//thrPool.schedule(renderTask);
			co_await render.render();
		}
	};
	auto r = root(thrPool);
	return thrPool.run(PoolCaps::UI);
}
