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
#include <iostream>
#include <swal/error.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <print>

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
using Microsoft::WRL::ComPtr;

namespace {
constexpr ivec2 wSize = {800, 600};
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
    void OnClose();
    void OnResize();
    void OnMouseMove(int x, int y);
    void OnKey(KeyboardKeyState, int);
    auto Load() -> task<void>;
    void InitD3D();

    dse::core::IOContext ctx;
    dse::core::SystemLoop uiLoop;
    Window window{uiLoop};
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
};

App::App(int argc, char *argv[])
{
    using enum dse::core::WindowFrameStyle;
    window.SetTitle(u8"dxtest");
    window.ChangeFrameStyle(Fixed);
    window.ResizeSurface(wSize);
    InitD3D();
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
    using enum dse::core::WindowShowCommand;
    window.Show(ShowNormal);
    return uiLoop.Run();
}

void App::OnClose()
{
    uiLoop.Stop(0);
}

void App::OnResize()
{}

void App::OnMouseMove(int x, int y)
{}

void App::OnKey(KeyboardKeyState state, int key)
{
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

void App::InitD3D()
{
    using swal::com_call;
    ComPtr<IDXGIFactory1> factory;
    ComPtr<IDXGIAdapter1> adapter;
    com_call(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    UINT i = 0;
    while (true) {
        context.Reset();
        device.Reset();
        try {
            com_call(factory->EnumAdapters1(i++, adapter.ReleaseAndGetAddressOf()));
        } catch (std::system_error& e) {
            break;
        }
        DXGI_ADAPTER_DESC desc;
        com_call(adapter->GetDesc(&desc));
        println(std::cout, "Adapter: {}", swal::wide_char_to_multibyte(1251, desc.Description));
        println(std::cout, "VendorId: {:x}", desc.VendorId);
        println(std::cout, "DeviceId: {:x}", desc.DeviceId);
        println(std::cout, "SubSysId: {:x}", desc.SubSysId);
        println(std::cout, "Revision: {:x}", desc.Revision);
        println(std::cout, "Dedicated video memory: {}", desc.DedicatedVideoMemory);
        println(std::cout, "Dedicated system memory: {}", desc.DedicatedSystemMemory);
        print(std::cout, "Trying to create D3D11Device: ");
        try {
            D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_10_0};
            com_call(D3D11CreateDevice(
                adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                NULL,
                D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                levels,
                std::size(levels),
                D3D11_SDK_VERSION,
                &device,
                nullptr,
                &context
            ));
        } catch (std::system_error& e) {
            println(std::cout, "FAILED ({})", e.what());
            continue;
        }
        println(std::cout, "OK");
    }
    flush(std::cout);
}

int main(int argc, char* argv[])
try {
    App app(argc, argv);
    return app.Run();
} catch (std::system_error& e) {
    std::cerr << e.what() << "\n";
    return e.code().value();
}
