#include <dse/util/coroutine.h>
#include <dse/core/Image.h>
#include <dse/core/ImageManipulator.h>

namespace dse::core {

void ImageDataDeleter::operator()(void *ptr) const
{
    ::operator delete(ptr);
}

Image::Image() :
    size()
{}

Image::Image(math::ivec2 size, bool linear) :
    size(size),
    data(::operator new(size.y() * size.x() * ImageManipulator::PixelSize)),
    linear(linear)
{
    ImageManipulator manip(data.get(), size);
    manip.Fill({0, 0}, size, 0);
}

math::ivec2 Image::Size()
{
    return size;
}

void* Image::Data()
{
    return data.get();
}

ImageManipulator Image::Manipulator()
{
    return { data.get(), size, linear };
}

void Image::LoadByProvider(ITextureDataProvider *provider, util::FunctionPtr<void (Image &&)> callback)
{
    struct LoadParameters {
        bool await_ready() { return false; }
        bool await_suspend(std::coroutine_handle<> handle)
        {
            provider->LoadParameters(&params, {handle.address(), +[](void* addr){
                std::coroutine_handle<>::from_address(addr)();
            }});
            return true;
        }
        auto await_resume() -> ITextureDataProvider::TextureParameters
        { return params; }
        ITextureDataProvider* provider;
        ITextureDataProvider::TextureParameters params;
    };
    struct LoadData {
        bool await_ready() { return false; }
        bool await_suspend(std::coroutine_handle<> handle)
        {
            provider->LoadData(data, 0, {handle.address(), +[](void* addr){
                std::coroutine_handle<>::from_address(addr)();
            }});
            return true;
        }
        void await_resume() {}
        ITextureDataProvider* provider;
        void* data;
    };
    struct Callback {
        bool await_ready() { return false; }
        bool await_suspend(std::coroutine_handle<> handle)
        {
            callback(std::move(image));
            return false;
        }
        void await_resume() {}
        util::FunctionPtr<void (Image &&)>& callback;
        Image& image;
    };
    auto coro = +[](ITextureDataProvider *provider, util::FunctionPtr<void (Image &&)> callback) -> util::Task<void> {
        auto params = co_await LoadParameters{ provider };
        Image image;
        image.linear = false;
        switch (params.format) {
        case ITextureDataProvider::BGRA8:
        case ITextureDataProvider::BGRX8:
            image.linear = true;
            [[fallthrough]];
        case ITextureDataProvider::BGRA8sRGB:
        case ITextureDataProvider::BGRX8sRGB:
            break;
        default:
            co_await Callback{ callback, image };
            co_return;
        }
        image.size = { params.width, params.height };
        image.data.reset(::operator new(params.height * params.width * ImageManipulator::PixelSize));
        co_await LoadData{ provider, image.data.get() };
        co_await Callback{ callback, image };
    };
    util::StartDetached(coro(provider, callback));
}

} // namespace dse::core
