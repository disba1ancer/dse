#include <dse/core/SystemLoop.h>
#include <dse/core/BasicBitmapLoader.h>
#include <dse/core/Image.h>
#include <dse/util/coroutine.h>
#include <dse/core/ImageManipulator.h>
#include <dse/core/ThreadPool.h>
#include <dse/core/Window.h>
#include <dse/core/FrameBuffer.h>
#include <dse/math/vmath.h>
#include <chrono>
#include <iostream>

using dse::core::Window;
using dse::core::FrameBuffer;
using dse::util::fn_tag;
using dse::core::ImageManipulator;
using dse::core::WindowShowCommand;
using dse::core::Image;
using dse::util::task;
using dse::core::BasicBitmapLoader;
using dse::math::vec4;
using dse::math::ivec2;
using dse::core::WndEvtDt;
using dse::core::KeyboardKeyState;
using namespace std::literals::chrono_literals;

namespace {
constexpr ivec2 wSize = {640, 480};
}

class App {
public:
    App(int argc, char* argv[]);
    App(App&&) = delete;
    App(const App&) = delete;
    App& operator=(App&&) = delete;
    App& operator=(const App&) = delete;
    ~App();
    int Run();
private:
    void Draw(void* buffer, dse::math::ivec2 size);
    void AfterRender();
    void OnClose();
    void OnResize();
    void OnMouseMove(int x, int y);
    void OnKey(KeyboardKeyState, int);
    auto Load() -> task<void>;
    void Step();

    dse::core::IOContext ctx;
    dse::core::SystemLoop uiLoop;
    Window window{uiLoop};
    FrameBuffer framebuffer{window};
    Image font;
    Image bg;
    static constexpr auto counterCount = 256;
    std::chrono::high_resolution_clock::duration times[counterCount] = {};
    int currentCounter = 0;
    ivec2 ballPos = {320, 240};
    ivec2 ballVelocity = {6, 2};
};

App::App(int argc, char *argv[])
{
    using enum dse::core::WindowFrameStyle;
    window.SetTitle(u8"Sample");
    window.ChangeFrameStyle(Fixed);
    window.ResizeSurface(wSize);
    using enum dse::core::WindowEvent;
    window.Register<Close>({*this, fn_tag<&App::OnClose>});
    window.Register<Resize>({*this, fn_tag<&App::OnResize>});
    window.Register<MouseMove>({*this, fn_tag<&App::OnMouseMove>});
    window.Register<Key>({*this, fn_tag<&App::OnKey>});
}

App::~App()
{
    using enum dse::core::WindowEvent;
    window.Unregister<Key>({*this, fn_tag<&App::OnKey>});
    window.Unregister<MouseMove>({*this, fn_tag<&App::OnMouseMove>});
    window.Unregister<Resize>({*this, fn_tag<&App::OnResize>});
    window.Unregister<Close>({*this, fn_tag<&App::OnClose>});
}

int App::Run()
{
    {
        auto task = Load();
        auto awaitable = task.operator co_await();
        if (!awaitable.await_ready()) {
            awaitable.await_suspend(std::noop_coroutine()).resume();
        }
        ctx.Run();
    }
    framebuffer.SetDrawCallback({*this, fn_tag<&App::Draw>});
    using enum dse::core::WindowShowCommand;
    window.Show(ShowNormal);
    auto next = std::chrono::high_resolution_clock::now();
    auto last = next;
    while (uiLoop.Poll()) {
        auto now = std::chrono::high_resolution_clock::now();
        if (now < next) {
            continue;
            std::this_thread::sleep_until(next);
        }
        next += 16667us;
        if (next < now) {
            next += (now - next) % 16667us;
        }
        times[currentCounter] = now - last;
        currentCounter = (currentCounter + 1) & (counterCount - 1);
        last = now;
        Step();
        framebuffer.Render(nullptr);
    }
    return uiLoop.Result();
}

void App::Draw(void* buffer, dse::math::ivec2 size)
{
    ImageManipulator manip(buffer, size, false);
    manip.Fill({0, 0}, wSize, 0x00000000);
//    manip.DrawRectFilled({0, 0}, size, {.0f, .0, 1.0, 1.f});
//    manip.BlendImage({256, 256}, wall.Size(), wall, {0, 0});
    // manip.DrawText({0, 0}, wSize, u8"Hello, world!", font, {8, 12});
    manip.Fill(ballPos - 8, {16, 16}, 0xFFFFFFFF);
    for (int i = 0; i < counterCount; ++i) {
        int us = std::chrono::duration_cast<std::chrono::microseconds>(times[i]).count();
        std::uint32_t color = 0xFF00FF00;
        if (i == currentCounter) {
            color = 0xFF008000;
        }
        manip.Fill({i, wSize.y() - us / 200}, {1, us / 200}, color);
    }
}

void App::AfterRender()
{}

void App::OnClose()
{
    uiLoop.Stop(0);
}

void App::OnResize()
{
    framebuffer.Render(nullptr);
}

void App::OnMouseMove(int x, int y)
{}

void App::OnKey(KeyboardKeyState state, int key)
{
    if (key == ' ' && state == KeyboardKeyState::DOWN) {
        ballPos = {320, 240};
        // ballVelocity = {6, 2};
    }
    if (key == 0x7A && state == KeyboardKeyState::DOWN) {
        if (window.IsFullscreen()) {
            window.Show(WindowShowCommand::ShowRestored);
        } else {
            window.Show(WindowShowCommand::ShowFullScreen);
        }
    }
    if (key == 'W' && state == KeyboardKeyState::DOWN) {
        window.Show(WindowShowCommand::ShowNormal);
    }
}

task<void> App::Load()
{
    struct LoadImage {
        bool await_ready()
        {
            return false;
        }
        bool await_suspend(std::coroutine_handle<> h)
        {
            handle = h;
            Image::LoadByProvider(provider, {*this, fn_tag<&LoadImage::Callback>});
            return true;
        }
        Image await_resume() {
            return std::move(image);
        }
        void Callback(Image&& img)
        {
            image = std::move(img);
            handle.resume();
        }
        dse::core::ITextureDataProvider* provider;
        Image image;
        std::coroutine_handle<> handle;
    };
    // BasicBitmapLoader loader(ctx, u8"assets/textures/font.bmp", false);
    // font = co_await LoadImage{&loader};
    co_await std::suspend_never{};
    struct UITransfer {
        bool await_ready()
        {
            return false;
        }
        bool await_suspend(std::coroutine_handle<> h)
        {
            handle = h;
            uiLoop.Post({*this, fn_tag<&UITransfer::Callback>});
            return true;
        }
        void await_resume() {
        }
        void Callback()
        {
            handle.resume();
        }
        dse::core::SystemLoop& uiLoop;
        std::coroutine_handle<> handle;
    };
}

void App::Step()
{
    ballPos += ballVelocity;
    auto t = ballPos - 8;
    if (t.x() < 0 || t.x() > 624) {
        ballVelocity.x() = -ballVelocity.x();
    }
    if (t.y() < 0 || t.y() > 464) {
        ballVelocity.y() = -ballVelocity.y();
    }
}

int main(int argc, char* argv[])
try {
    App app(argc, argv);
    return app.Run();
} catch (std::system_error& e) {
    std::cerr << e.what() << "\n";
    return e.code().value();
}
