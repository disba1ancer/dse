/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include <dse/util/coroutine.h>
#include <dse/core/Window.h>
#include <cstdio>
#include <cstdint>
#include <dse/renders/RenderOpenGL31.h>
#include <dse/util/functional.h>
#include <dse/core/Scene.h>
#include <dse/core/Cube.h>
#include <dse/core/Object.h>
#include <cmath>
#include <dse/math/qmath.h>
#include <dse/math/constants.h>
#include <dse/core/Camera.h>
#include <dse/core/mcursor.h>
#include <dse/core/Material.h>
#include <functional>
#include <algorithm>
#include <dse/core/ThreadPool.h>
#include <dse/core/File.h>
#include <dse/util/scope_exit.h>
#include <dse/util/execution.h>
#include <dse/core/BasicBitmapLoader.h>

#include <swal/error.h>
#include <string>

using namespace std::string_literals;
using dse::core::Window;
using dse::core::WindowShowCommand;
using dse::core::KeyboardKeyState;
using dse::core::WndEvtDt;
using dse::core::PntEvtDt;
using dse::ogl31rbe::RenderOpenGL31;
using dse::core::Cube;
using dse::core::Scene;
using dse::core::Object;
using dse::core::Camera;
using dse::core::SetMouseCursorPosWndRel;
using dse::core::Material;
using dse::core::ThreadPool;
using dse::core::PoolCaps;
using dse::core::File;
using dse::core::OpenMode;
using dse::core::BasicBitmapLoader;

//ExecutionThread mainThread;
ThreadPool thrPool;

auto mainTask(Window& window, RenderOpenGL31& render) -> dse::util::Task<void>;

void selfCycle(void*) {
	std::printf("cycle\n");
	thrPool.Schedule({nullptr, selfCycle});
}

int main(int argc, char* argv[])
{
	Window window;
	RenderOpenGL31 render(window);
	window.Show();
	SetMouseCursorPosWndRel(window.Size() / 2, window);
	auto r = mainTask(window, render);
	thrPool.Schedule(dse::util::FunctionPtr<void()>(&r, [](void*p){
		static_cast<decltype(r)*>(p)->Resume();
	}));
//	thrPool.Schedule(selfCycle);
	return thrPool.Run(PoolCaps::UI);
}

auto mainTask(Window& window, RenderOpenGL31& render) -> dse::util::Task<void>
{
	using namespace dse::math;
	/*File file(thrPool, u8"test.txt", OpenMode::Read);
	std::byte buf[4096];
	if (file.IsValid()) {
		auto [transfered, error] = co_await file.ReadAsync(buf, std::size(buf));
		std::printf("%.*s\n", int(transfered), reinterpret_cast<char*>(buf));
	} else {
		std::printf("%s%s\n", "ERROR: ", file.Status().message().data());
	}
	std::fflush(stdout);*/

	Camera cam;
	Scene scene;
	Cube cubeMesh;
	BasicBitmapLoader texture(thrPool, u8"assets/textures/wall_test.bmp");
	Material mat(&texture, dse::math::vec4{.2f, .8f, .4f, .0f});
	auto cube1 = scene.createObject(Object(&cubeMesh));
	auto cube2 = scene.createObject(Object(&cubeMesh));
	cube1->SetPos({-1.f, -1.f, -1.f});
	//cube1->setScale({.25f, .25f, .25f});
	cube1->SetMaterial(0, &mat);
	cube2->SetPos({1.f, 1.f, 1.f});
	//cube2->setScale({.25f, .25f, .25f});
	cube2->SetMaterial(0, &mat);
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
			if (cmd == KeyboardKeyState::DOWN) {
				thrPool.Stop();
			}
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
	auto lastSecondEnd = lastFrameEnd + std::chrono::seconds(1);
	while (true) {
		using namespace dse::math;
		auto angle = 1.f;
		auto axe1 = norm(-vec3{1.f, 1.f, 1.f}) * std::sin(PI * angle / 360.f);
		auto cs1 = std::cos(PI * angle / 360.f);
		auto rslt = norm(qmul(vec4{axe1.x(), axe1.y(), axe1.z(), cs1}, cube1->GetQRot()));
		cube1->SetQRot(rslt);
		//cube2->setQRot(rslt);
		if (spd != 0.f || sdspd != 0.f) {
			auto movement = norm(vec3{0.f, 0.f, -1.f} * spd + vec3{1.f, 0.f, 0.f} * sdspd);
			cam.setPos(cam.getPos() + vecrotquat(movement, cam.getRot()));
		}
		constexpr float sens = 30.f / 65536.f;
		pitch = std::clamp(pitch - float(moffset.y()) * sens, 0.f, PI);
		yaw -= float(moffset.x()) * sens;
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
		if ((lastSecondEnd - lastFrameEnd) < frameDuration) {
			lastFrameEnd = lastSecondEnd;
			lastSecondEnd += std::chrono::seconds(1);
		}
//			lastFrameEnd = cur;
		auto ms = std::chrono::duration_cast<std::chrono::microseconds>(curDur);
		auto count = ms.count();
		std::printf("                \r%lli\r", count);
		while (std::chrono::steady_clock::now() < lastFrameEnd) ;
	}
}
