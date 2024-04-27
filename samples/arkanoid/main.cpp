#include <dse/core/BasicBitmapLoader.h>
#include <dse/core/Image.h>
#include <dse/util/coroutine.h>
#include <dse/core/ImageManipulator.h>
#include <dse/core/ThreadPool.h>
#include <dse/core/Window.h>
#include <dse/core/FrameBuffer.h>
#include <dse/math/vmath.h>

using dse::core::Window;
using dse::core::FrameBuffer;
using dse::util::fnTag;
using dse::util::FunctionPtr;
using dse::core::ImageManipulator;
using dse::core::WindowShowCommand;
using dse::core::Image;
using dse::util::Task;
using dse::core::BasicBitmapLoader;
using dse::math::vec4;
using dse::math::ivec2;

namespace {
constexpr ivec2 wSize = {640, 480};
}

class App {
public:
    App(int argc, char* argv[]);
    int Run();
private:
    void Draw(void* buffer, dse::math::ivec2 size);
    void AfterRender();
    void OnClose(dse::core::WndEvtDt);
    void OnResize(dse::core::WndEvtDt, int w, int h, WindowShowCommand);
    void OnMouseMove(dse::core::WndEvtDt, int x, int y);
    auto CoRun() -> Task<void>;

    dse::core::ThreadPool tpool;
    Window window;
    FrameBuffer framebuffer;
    Image font;
};

App::App(int argc, char *argv[]) :
    tpool(2),
    framebuffer(window)
{
    framebuffer.SetDrawCallback({*this, fnTag<&App::Draw>});
    window.Resize(wSize);
}

int App::Run()
{
    auto closeCon = window.SubscribeCloseEvent(FunctionPtr{*this, fnTag<&App::OnClose>});
    auto resizeCon = window.SubscribeResizeEvent(FunctionPtr{*this, fnTag<&App::OnResize>});
    auto mMoveCon = window.SubscribeMouseMoveEvent(FunctionPtr{*this, fnTag<&App::OnMouseMove>});
    auto task = CoRun();
    tpool.Schedule(task);
    return tpool.Run(dse::core::PoolCaps::UI);
}

void App::Draw(void* buffer, dse::math::ivec2 size)
{
    ImageManipulator manip(buffer, size, false);
    manip.Fill({16, 16}, {128, 128}, 0x00000000);
//    manip.DrawRectFilled({0, 0}, size, {.0f, .0, 1.0, 1.f});
//    manip.BlendImage({256, 256}, wall.Size(), wall, {0, 0});
    manip.DrawText({0, 0}, wSize, u8"Hello, world!", font, {8, 12});
}

void App::AfterRender()
{}

void App::OnClose(dse::core::WndEvtDt)
{
    tpool.Stop();
}

void App::OnResize(dse::core::WndEvtDt, int w, int h, WindowShowCommand)
{
    framebuffer.Render(nullptr);
}

void App::OnMouseMove(dse::core::WndEvtDt, int x, int y)
{}

Task<void> App::CoRun()
{
    struct LoadImage {
        bool await_ready() { return false; }
        bool await_suspend(std::coroutine_handle<> h)
        {
            handle = h;
            Image::LoadByProvider(provider, {*this, fnTag<&LoadImage::Callback>});
            return true;
        }
        void await_resume() {}
        void Callback(Image&& img)
        {
            image = std::move(img);
            handle.resume();
        }
        dse::core::ITextureDataProvider* provider;
        Image& image;
        std::coroutine_handle<> handle;
    };
    BasicBitmapLoader loader(tpool, u8"assets/textures/font.bmp", false);
    co_await LoadImage{ &loader, font };
    window.Show();
    framebuffer.Render(nullptr);
}

int main(int argc, char* argv[])
{
    App app(argc, argv);
    return app.Run();
}
