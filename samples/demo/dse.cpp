/*
 * dse.cpp
 *
 *  Created on: 25 дек. 2019 г.
 *      Author: disba1ancer
 */

#include <dse/core/Window.h>
#include <dse/renders/RenderOpenGL31.h>
#include <dse/core/Cube.h>
#include <dse/core/Sphere.h>
#include <dse/math/qmath.h>
#include <dse/math/constants.h>
#include <dse/core/mcursor.h>
#include <dse/core/BasicBitmapLoader.h>

#include <print>

using dse::core::Window;
using dse::core::KeyboardKeyState;
using dse::ogl31rbe::RenderOpenGL31;
using dse::core::Cube;
using dse::core::Sphere;
using dse::core::IScene;
using dse::core::ISceneObject;
using dse::core::IMesh2;
using dse::core::IMaterial;
using dse::core::ITexture;
using dse::core::Camera;
using dse::core::SystemLoop;
using dse::core::IOContext;
using dse::core::BasicBitmapLoader;
using dse::util::fn_tag;
using dse::core::IResourceManager;
using dse::util::event_manager;
using dse::core::SceneEvent;
using dse::core::ISceneVisitor;
using dse::core::embedded_event_manager;
using namespace dse::math;

class App : embedded_event_manager<IScene, SceneEvent> {
public:
    int Run();
private:
    void OnClose();
    void OnKey(KeyboardKeyState cmd, int key);
    void OnMouseMove(int x, int y);
    void DoStep();
    void VisitObjects(ISceneVisitor &visitor);
    void VisitLights(ISceneVisitor &visitor);
    void VisitAll(ISceneVisitor &visitor);

    struct Object : ISceneObject
    {
        Object(vec3 p, vec4 r, IMesh2& m, IMaterial& mat) : position(p), rotation(r), mesh(m), material(mat) {}
        vec3 GetPosition() override;
        vec4 GetRotation() override;
        vec3 GetScale() override;
        auto GetMesh() -> IMesh2& override;
        auto GetMaterial(int slotNum) -> IMaterial& override;

        vec3 position;
        vec4 rotation;
        IMesh2& mesh;
        IMaterial& material;
    };

    struct Material : IMaterial
    {
        Material(const vec4& c, ITexture& d, ITexture& n) : color(c), diffuse(d), normalMap(n) {}
        vec4 GetColor() override;
        auto GetDiffuseTexture() -> ITexture& override;
        auto GetNormalMapTexture() -> ITexture& override;
        auto GetResourceManager() -> IResourceManager& override;
        vec4 color;
        ITexture& diffuse;
        ITexture& normalMap;
    };

    IOContext context;
    SystemLoop loop;
    Window window{loop};
    RenderOpenGL31 render{this->window};
    BasicBitmapLoader texture{this->context, u8"assets/textures/wall_test.bmp"};
    BasicBitmapLoader mapTex{this->context, u8"assets/textures/earth.bmp"};
    BasicBitmapLoader mapNorm{this->context, u8"assets/textures/earthnormalfull.bmp", true};
    Camera cam;
    Cube cubeMesh;
    Sphere sphereMesh{32, 48};
    Material mat{{1.f, 1.f, 1.f, 1.f}, texture, mapNorm};
    Material mapMat{{1.f, 1.f, 1.f, 1.f}, mapTex, mapNorm};
    Object cube{{-1.f, -1.f, -1.f}, {0.f, 0.f, 0.f, 1.f}, cubeMesh, mat};
    Object sphere{{1.f, 1.f, 1.f}, {0.f, 0.f, 0.f, 1.f}, sphereMesh, mapMat};
    float pitch = dse::math::PI * 0.5, yaw = 0.f;
    float spd = 0.f, sdspd = 0.f;
    ivec2 moffset = { 0, 0 };
    std::chrono::steady_clock::time_point lastFrameEnd = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastSecondEnd = lastFrameEnd + std::chrono::seconds(1);
    using enum dse::core::WindowEvent;
    using howner = dse::core::Window::handle_owner;
    howner hOnClose = window.SubscribeEvent<Close>({*this, fn_tag<&App::OnClose>});
    howner hOnKey = window.SubscribeEvent<Key>({*this, fn_tag<&App::OnKey>});
    howner hOnMouseMove = window.SubscribeEvent<MouseMove>({*this, fn_tag<&App::OnMouseMove>});
};

int App::Run()
{
    window.Show();
    SetMouseCursorPosWndRel(window.SurfaceSize() / 2, window);
    render.SetScene(this);
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
    case 122:
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
    auto rslt = norm(qmul(vec4{axe1.x(), axe1.y(), axe1.z(), cs1}, cube.rotation));
    cube.rotation = rslt;
    sphere.rotation = rslt;
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
    eventManager.send<SceneEvent::ObjectModified>((IScene&)*this, (ISceneObject&)cube);
    eventManager.send<SceneEvent::ObjectModified>((IScene&)*this, (ISceneObject&)sphere);

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

void App::VisitObjects(dse::core::ISceneVisitor &visitor)
{
    visitor.Visit(cube);
    visitor.Visit(sphere);
}

void App::VisitLights(dse::core::ISceneVisitor &visitor)
{}

void App::VisitAll(dse::core::ISceneVisitor &visitor)
{
    VisitObjects(visitor);
}

int main(int argc, char* argv[])
{
    App app;
    return app.Run();
}

vec3 App::Object::GetPosition()
{
    return position;
}

vec4 App::Object::GetRotation()
{
    return rotation;
}

vec3 App::Object::GetScale()
{
    return {1.f, 1.f, 1.f};
}

auto App::Object::GetMesh() -> IMesh2&
{
    return mesh;
}

auto App::Object::GetMaterial(int slotNum) -> IMaterial&
{
    return material;
}

vec4 App::Material::GetColor()
{
    return color;
}

auto App::Material::GetDiffuseTexture() -> ITexture&
{
    return diffuse;
}

auto App::Material::GetNormalMapTexture() -> ITexture&
{
    return normalMap;
}

auto App::Material::GetResourceManager() -> IResourceManager&
{
    return dse::core::StaticResourceManager::instance;
}
