/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include <dse/util/coroutine.h>
#include <dse/core/Window.h>
#include <cstdio>
#include <dse/renders/RenderOpenGL31.h>
#include <dse/util/functional.h>
#include <dse/core/Scene.h>
#include <dse/core/Cube.h>
#include <dse/core/Sphere.h>
#include <dse/core/Object.h>
#include <cmath>
#include <dse/math/qmath.h>
#include <dse/math/constants.h>
#include <dse/core/Camera.h>
#include <dse/core/mcursor.h>
#include <dse/core/Material.h>
#include <algorithm>
#include <dse/core/ThreadPool.h>
#include <dse/core/File.h>
#include <dse/util/scope_exit.h>
#include <dse/util/execution.h>
#include <dse/core/BasicBitmapLoader.h>

#include <print>

using namespace std::string_literals;
using dse::core::Window;
using dse::core::WindowShowCommand;
using dse::core::KeyboardKeyState;
using dse::ogl31rbe::RenderOpenGL31;
using dse::core::Cube;
using dse::core::Sphere;
using dse::core::Scene;
using dse::core::Object;
using dse::core::Camera;
using dse::core::SetMouseCursorPosWndRel;
using dse::core::Material;
using dse::core::ThreadPool;
using dse::core::SystemLoop;
using dse::core::IOContext;
using dse::core::PoolCaps;
using dse::core::File;
using dse::core::OpenMode;
using dse::core::BasicBitmapLoader;
using dse::util::fn_tag;
using namespace dse::math;

class App {
public:
    int Run();
private:
    void OnClose();
    void OnKey(KeyboardKeyState cmd, int key);
    void OnMouseMove(int x, int y);
    void DoStep();

    IOContext context;
    SystemLoop loop;
    Window window{loop};
    RenderOpenGL31 render{this->window};
    BasicBitmapLoader texture{this->context, u8"assets/textures/wall_test.bmp"};
    BasicBitmapLoader mapTex{this->context, u8"assets/textures/earth.bmp"};
    BasicBitmapLoader mapNorm{this->context, u8"assets/textures/earthnormalfull.bmp", true};
    Camera cam;
    Scene scene;
    Cube cubeMesh;
    Sphere sphereMesh{32, 48};
    Material mat{&texture};
    Material mapMat{&mapTex};
    Object& cube = *scene.createObject( Object(&cubeMesh));
    Object& sphere = *scene.createObject(Object(&sphereMesh));
    float pitch = dse::math::PI * 0.5, yaw = 0.f;
    float spd = 0.f, sdspd = 0.f;
    ivec2 moffset = { 0, 0 };
    std::chrono::steady_clock::time_point lastFrameEnd = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastSecondEnd = lastFrameEnd + std::chrono::seconds(1);
    using enum dse::core::WindowEvent;
    using howner = dse::util::handle_owner<dse::core::WindowEventHandle>;
    howner hOnClose = window.Register<Close>({*this, fn_tag<&App::OnClose>});
    howner hOnKey = window.Register<Key>({*this, fn_tag<&App::OnKey>});
    howner hOnMouseMove = window.Register<MouseMove>({*this, fn_tag<&App::OnMouseMove>});
};

int App::Run()
{
    window.Show();
    SetMouseCursorPosWndRel(window.SurfaceSize() / 2, window);
    mapMat.SetNormalMap(&mapNorm);
    cube.SetPos({-1.f, -1.f, -1.f});
    //cube1.setScale({.25f, .25f, .25f});
    cube.SetMaterial(0, &mat);
    sphere.SetPos({1.f, 1.f, 1.f});
    //cube2.setScale({.25f, .25f, .25f});
    sphere.SetMaterial(0, &mapMat);
    render.SetScene(scene);
    cam.setPos({0.f, -4.f, 0.f});
    cam.setRot({std::sqrt(2.f) * 0.5f, 0.f, 0.f, std::sqrt(2.f) * 0.5f});
    cam.setNear(.03125f);
    cam.setFar(1024.f);
    render.SetCamera(cam);
    loop.Periodic(10000, {*this, fn_tag<&App::DoStep>}).detach();
    return loop.Run();
}

void App::OnClose()
{
    loop.Stop(0);
}

void App::OnKey(KeyboardKeyState cmd, int key)
{
    constexpr auto speed = 1.f / 8;
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
            loop.Stop(0);
        }
        break;
    case 'F':
        if (cmd == KeyboardKeyState::UP) {
            break;
        }
        if (window.IsFullscreen()) {
            window.Show(dse::core::WindowShowCommand::ShowRestored);
        } else {
            window.Show(dse::core::WindowShowCommand::ShowFullScreen);
        }
        break;
    default:
        std::printf("%i %i\n", static_cast<int>(cmd), key);
        std::fflush(stdout);
    }
}

void App::OnMouseMove(int x, int y)
{
    auto center = window.SurfaceSize() / 2;
    moffset += ivec2{x, y} - center;
    SetMouseCursorPosWndRel(center, window);
}

void App::DoStep()
{
    context.Poll();
    using namespace dse::math;
    auto angle = 1.f;
    auto axe1 = norm(vec3{0.f, 0.f, 1.f}) * std::sin(PI * angle / 360.f);
    auto cs1 = std::cos(PI * angle / 360.f);
    auto rslt = norm(qmul(vec4{axe1.x(), axe1.y(), axe1.z(), cs1}, cube.GetQRot()));
    cube.SetQRot(rslt);
    sphere.SetQRot(rslt);
    if (spd != 0.f || sdspd != 0.f) {
        auto movement = vec3{0.f, 0.f, -1.f} * spd + vec3{1.f, 0.f, 0.f} * sdspd;
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

    render.Render();

    constexpr auto frameDuration = std::chrono::nanoseconds(1000000000 / 60);
    auto cur = std::chrono::steady_clock::now();
    auto curDur = cur - lastFrameEnd;
    //        auto frames = (curDur / frameDuration) + 1;
    //        lastFrameEnd += frames * frameDuration;
    //        if ((lastSecondEnd - lastFrameEnd) < frameDuration) {
    //            lastFrameEnd = lastSecondEnd;
    //            lastSecondEnd += std::chrono::seconds(1);
    //        }
    lastFrameEnd = cur;
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(curDur);
    auto count = ms.count();
    std::print("                \r{}\r", count);
    //        while (std::chrono::steady_clock::now() < lastFrameEnd) ;
}

int main(int argc, char* argv[])
{
    App app;
    return app.Run();
}
