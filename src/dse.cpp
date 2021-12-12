/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include "util/coroutine.h"
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
#include "util/execution.h"

#include <swal/error.h>
#include <string>

using namespace std::string_literals;
using dse::os::Window;
using dse::os::WindowShowCommand;
using dse::os::KeyboardKeyState;
using dse::os::WndEvtDt;
using dse::os::PntEvtDt;
using dse::renders::RenderOpenGL31;
using dse::util::StaticMemFn;
using dse::scn::Cube;
using dse::scn::Scene;
using dse::scn::Object;
using dse::scn::Camera;
using dse::os::SetMouseCursorPosWndRel;
using dse::scn::Material;
using dse::core::ThreadPool;
using dse::core::PoolCaps;
using dse::core::File;
using dse::core::OpenMode;

//ExecutionThread mainThread;
ThreadPool thrPool(1);

int main(int argc, char* argv[]) {
	Window window;
	RenderOpenGL31 render(window);
	window.Show();
	SetMouseCursorPosWndRel(window.Size() / 2, window);
	auto root = [&window, &render, argc, argv]() -> dse::util::Task<void> {
		using namespace dse::math;
		File file(thrPool, u8"test.txt", OpenMode::READ);
		std::byte buf[4096];
		if (file.IsValid()) {
			auto [transfered, error] = co_await file.ReadAsync(buf, std::size(buf));
			std::printf("%.*s\n", int(transfered), reinterpret_cast<char*>(buf));
		} else {
			std::printf("%s%s\n", "ERROR: ", file.Status().message().data());
		}
		std::fflush(stdout);

		Camera cam;
		Scene scene;
		Cube cubeMesh;
		Material mat(dse::math::vec4{.2f, .8f, .4f, .0f});
		auto cube1 = scene.createObject(Object(&cubeMesh));
		auto cube2 = scene.createObject(Object(&cubeMesh));
		cube1->setPos({-1.f, -1.f, -1.f});
		//cube1->setScale({.25f, .25f, .25f});
		cube1->setMaterial(0, &mat);
		cube2->setPos({1.f, 1.f, 1.f});
		//cube2->setScale({.25f, .25f, .25f});
		cube2->setMaterial(0, &mat);
		render.SetScene(scene);
		cam.setPos({0.f, -4.f, 0.f});
		cam.setRot({std::sqrt(2.f) * 0.5f, 0.f, 0.f, std::sqrt(2.f) * 0.5f});
		cam.setNear(.03125f);
		cam.setFar(1024.f);
		render.SetCamera(cam);
		auto closeCon = window.SubscribeCloseEvent([](WndEvtDt){
			thrPool.Stop();
		});
		float pitch = dse::math::PI * 0.5, yaw = 0.f;
		float spd = 0.f, sdspd = 0.f;
		auto keyCon = window.SubscribeKeyEvent([&spd, &sdspd](WndEvtDt, KeyboardKeyState cmd, int key){
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
				thrPool.Stop();
				break;
			default:
				std::printf("%i %i\n", static_cast<int>(cmd), key);
				std::fflush(stdout);
			}
		});
		ivec2 moffset = { 0, 0 };
		auto mmoveCon = window.SubscribeMouseMoveEvent([&window, &moffset](WndEvtDt, int x, int y) {
			auto center = window.Size() / 2;
			moffset += ivec2{x, y} - center;
			SetMouseCursorPosWndRel(center, window);
		});
		/*auto sizeCon = window.subscribeResizeEvent([](WndEvtDt, int, int, WindowShowCommand) {
			mainThread.yieldTasks();
		});
		mainThread.addTask(dse::os::nonLockLoop());*/
		auto lastFrameEnd = std::chrono::steady_clock::now();
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
			constexpr float sens = 30.f / 65536.f;
			pitch = std::clamp(pitch - moffset.y() * sens, 0.f, PI);
			yaw -= moffset.x() * sens;
			auto pitchHalf = pitch * .5f;
			auto yawHalf = yaw * .5f;
			auto camrot = qmul(vec4{ 0, 0, std::sin(yawHalf), std::cos(yawHalf) }, vec4{ std::sin(pitchHalf), 0, 0, std::cos(pitchHalf) });
			cam.setRot(camrot);
			moffset = vec2{0, 0};
			//renderTask.reset(make_handler<&RenderOpenGL31::renderTask>(render));
			//renderTask.then([&stepTask]{ thrPool.schedule(stepTask); });
			//thrPool.schedule(renderTask);

			co_await render.Render();

			constexpr auto frameDuration = std::chrono::nanoseconds(1000000000 / 60);
			auto cur = std::chrono::steady_clock::now();
			auto curDur = cur - lastFrameEnd;
			auto frames = (curDur / frameDuration) + 1;
			lastFrameEnd += frames * frameDuration;
//			lastFrameEnd = cur;
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(curDur);
			auto count = ms.count();
			std::printf("                \r%lli\r", count);
			while (std::chrono::steady_clock::now() < lastFrameEnd) ;
		}
	};
	auto r = root();
	thrPool.Schedule(dse::util::FunctionPtr<void()>(&r, [](void*p){
		static_cast<decltype(r)*>(p)->Resume();
	}));
	return thrPool.Run(PoolCaps::UI);
}
