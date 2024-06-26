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

static char8_t text[] = u8"Vivamus ut tincidunt dui.\nAenean facilisis nibh mi, at "
                        "venenatis nibh sagittis sed.\nSed scelerisque ipsum vitae erat "
                        "auctor pretium.\nNam auctor ante sit amet eros lacinia, vitae "
                        "accumsan enim maximus.\nNunc dignissim, lacus eu feugiat "
                        "interdum, erat leo imperdiet lectus, sed semper ligula quam "
                        "accumsan urna.\nNulla aliquam mi maximus, rhoncus diam ut, "
                        "hendrerit odio.\nSuspendisse lacus massa, congue sit amet lacus "
                        "id, lobortis congue purus.\nCurabitur elementum quis magna non "
                        "convallis.";

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
    Image wall;
    ivec2 mOldPos = {0, 0};
    ivec2 mPos = {0, 0};
};

App::App(int argc, char *argv[]) :
    tpool(2),
    framebuffer(window)
{
    framebuffer.SetDrawCallback({*this, fnTag<&App::Draw>});
    window.Resize({640, 480});
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
//    const std::uint32_t colors[] = {
//        manip.ToRawColor({.25f, .25f, .25f, 1.f}),
//        manip.ToRawColor({.75f, .75f, .75f, 1.f})
//    };
//    for (int y = 0; y < size.y(); ++y) {
//        auto yIndex = (y >> 3) & 1;
//        for (int x = 0; x < size.x(); ++x) {
//            std::size_t colorIndex = ((x >> 3) & 1) ^ yIndex;
//            manip[y][x] = colors[colorIndex];
//        }
//    }
    auto checkFunc = [](std::uint32_t dstColor) -> bool
    {
        return ((dstColor >> 24) & 0xFF) == 0xFF;
    };
    auto blendFunc = [&manip](std::uint32_t dstColorR, vec4 srcColor) -> std::uint32_t
    {
        auto a = srcColor.w();
        srcColor *= vec4{a, a, a, 1.f};
        if (((dstColorR >> 24) & 0xFF) == 0) {
            return manip.ToRawColor(srcColor);
        }
        auto dstColor = manip.FromRawColor(dstColorR);
        return manip.ToRawColor(dstColor + srcColor * (1 - dstColor.w()));
    };
    manip.Fill({16, 16}, {128, 128}, 0x00000000);
    auto old1 = manip.SetBlendFunc(blendFunc);
    auto old2 = manip.SetEarlyBlendFunc(checkFunc);
//    manip.DrawRectFilled({0, 0}, size, {.0f, .0, 1.0, 1.f});
//    manip.BlendImage({256, 256}, wall.Size(), wall, {0, 0});
    dse::math::ivec2 rb = {16, 16};
    dse::math::ivec2 rs = {512, 384};
    manip.DrawText(rb, rs, text, font, {8, 12});
    manip.DrawRect(rb - 1, rs + 2, 1, {0.f, .25f, .5f, 1.f});
    manip.DrawRectFilled(rb, rs, {0.f, 0.5f, 1.f, 1.f});
    manip.SetEarlyBlendFunc(old2);
    manip.SetBlendFunc(old1);
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
    BasicBitmapLoader loader2(tpool, u8"assets/textures/wall_alpha.bmp", false);
    co_await LoadImage{ &loader, font };
    co_await LoadImage{ &loader2, wall };
    window.Show();
    framebuffer.Render(nullptr);
}

int main(int argc, char* argv[])
{
    App app(argc, argv);
    return app.Run();
}
