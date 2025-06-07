#include <dse/core/SystemLoop.h>
#include <dse/core/BasicBitmapLoader.h>
#include <dse/core/Image.h>
#include <dse/util/coroutine.h>
#include <dse/core/ImageManipulator.h>
#include <dse/core/ThreadPool.h>
#include <dse/core/Window.h>
#include <dse/core/FrameBuffer.h>
#include <dse/math/vmath.h>
#include <dse/math/mat.h>
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
    // static void OnClose(App&&);
    void OnResize();
    void OnMouseMove(int x, int y);
    void OnKey(KeyboardKeyState, int);
    auto Load() -> task<void>;
    void Step();
    using Rect = dse::math::vec<ivec2, 2>;
    Rect GetCollideRegion(ivec2 vel);
    int GetBrick(ivec2 pos);

    dse::core::IOContext ctx;
    dse::core::SystemLoop uiLoop;
    Window window{uiLoop};
    FrameBuffer framebuffer{window};
    Image font;
    Image bg;
    static constexpr auto counterCount = 256;
    static constexpr ivec2 ballSize = {16, 16};
    static constexpr ivec2 brickSize = {32, 16};
    static constexpr ivec2 boardSize = {20, 30};
    std::chrono::high_resolution_clock::duration times[counterCount] = {};
    int currentCounter = 0;
    Rect ball = Rect{{{0, 0}, ballSize}} + wSize / 2;
    ivec2 ballPos = (wSize - ballSize) / 2;
    ivec2 ballVelocity = {-6, 2};
};

App::App(int argc, char *argv[])
{
    using enum dse::core::WindowFrameStyle;
    window.SetTitle(u8"Arkanoid");
    window.ChangeFrameStyle(Fixed);
    window.ResizeSurface(wSize);
    using enum dse::core::WindowEvent;
    window.SubscribeEvent<Close>({*this, fn_tag<&App::OnClose>}).detach();
    window.SubscribeEvent<Resize>({*this, fn_tag<&App::OnResize>}).detach();
    window.SubscribeEvent<MouseMove>({*this, fn_tag<&App::OnMouseMove>}).detach();
    window.SubscribeEvent<Key>({*this, fn_tag<&App::OnKey>}).detach();
}

App::~App()
{}

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
    auto next = std::chrono::steady_clock::now();
    auto last = next;
    auto timer_func = [&]{
        auto now = std::chrono::steady_clock::now();
        times[currentCounter] = now - last;
        currentCounter = (currentCounter + 1) & (counterCount - 1);
        last = now;
        Step();
        framebuffer.Render();
    };
    uiLoop.Periodic(10000, timer_func).detach();
    return uiLoop.Run();
}

void App::Draw(void* buffer, dse::math::ivec2 size)
{
    ImageManipulator manip(buffer, size, false);
    manip.Fill({0, 0}, wSize, 0xFF000000);
//    manip.DrawRectFilled({0, 0}, size, {.0f, .0, 1.0, 1.f});
//    manip.BlendImage({256, 256}, wall.Size(), wall, {0, 0});
    // manip.DrawText({0, 0}, wSize, u8"Hello, world!", font, {8, 12});
    manip.Fill(ballPos, ballSize, 0xFFFFFFFF);
    for (int i = 0; i < counterCount; ++i) {
        int us = std::chrono::duration_cast<std::chrono::microseconds>(times[i]).count();
        std::uint32_t color = 0xFF00FF00;
        if (i == currentCounter) {
            color = 0xFF008000;
        }
        manip.Fill({i, wSize.y() - us / 400}, {1, us / 400}, color);
    }
    manip.Fill({0, wSize.y() - 10000 / 400}, {counterCount, 1}, 0xFF808080);
    manip.Fill({0, wSize.y() - 16667 / 400}, {counterCount, 1}, 0xFF808080);
    manip.Fill({0, wSize.y() - 20000 / 400}, {counterCount, 1}, 0xFF808080);
    manip.Fill({0, wSize.y() - 30000 / 400}, {counterCount, 1}, 0xFF808080);
}

void App::AfterRender()
{}

void App::OnClose()
{
    uiLoop.Stop(0);
}

// void App::OnClose(App&& app)
// {
//     app.uiLoop.Stop(0);
// }

void App::OnResize()
{
    framebuffer.Render();
}

void App::OnMouseMove(int x, int y)
{}

void App::OnKey(KeyboardKeyState state, int key)
{
    if (key == ' ' && state == KeyboardKeyState::DOWN) {
        ballPos = {320, 180};
        ballVelocity = {-6, 2};
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
        dse::core::ITexture* provider;
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

template <typename T>
static bool between(const T& val, const T& bound1, const T& bound2)
{
    using std::min;
    using std::max;
    auto bmin = min(bound1, bound2);
    auto bmax = max(bound1, bound2);
    return (bmin < val && val < bmax) || val == bound1;
}

void App::Step()
{
    using std::min;
    using std::max;
    using std::abs;
    Rect boardBounds{{{0, 0}, wSize}};
    auto curVel = ballVelocity;
    do {
        auto coMul = abs(curVel["yx"]);
        auto dist = coMul.x() * coMul.y();
        auto cr = GetCollideRegion(curVel);
        ivec2 mul2 = {1, 1};
        Rect ballRect = {ballPos, ballPos + ballSize};
        for (int j = 0; j < 2; j++) {
            auto expr = cr[0][j] <=> boardBounds[0][j];
            if (expr >= 0) {
                continue;
            }
            auto d = abs(ballRect[0][j] - boardBounds[0][j]) * coMul[j];
            if (d >= dist) {
                continue;
            }
            dist = d;
            mul2 = {-1 + j * 2, 1 - j * 2};
        }
        for (int j = 0; j < 2; j++) {
            auto expr = cr[1][j] <=> boardBounds[1][j];
            if (expr <= 0) {
                continue;
            }
            auto d = abs(ballRect[1][j] - boardBounds[1][j]) * coMul[j];
            if (d >= dist) {
                continue;
            }
            dist = d;
            mul2 = {-1 + j * 2, 1 - j * 2};
        }
        ballPos += (sign(curVel) * dist) / coMul;
        curVel -= (sign(curVel) * dist) / coMul;
        curVel *= mul2;
        ballVelocity *= mul2;
    } while (curVel != ivec2{});
}

auto App::GetCollideRegion(ivec2 vel) -> Rect
{
    using std::swap;
    Rect result = {ballPos, ballPos + vel};
    if (result[0].x() > result[1].x()) {
        swap(result[0].x(), result[1].x());
    }
    if (result[0].y() > result[1].y()) {
        swap(result[0].y(), result[1].y());
    }
    result[1] += ballSize;
    return result;
}

int App::GetBrick(ivec2 pos)
{
    auto x = 0 <= pos.x() and pos.x() < boardSize.x();
    auto y = 0 <= pos.y() and pos.y() < boardSize.y();
    return -1 * !(x and y);
}

int main(int argc, char* argv[])
try {
    App app(argc, argv);
    return app.Run();
} catch (std::system_error& e) {
    std::cerr << e.what() << "\n";
    return e.code().value();
}
