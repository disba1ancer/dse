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
            this->handle = handle;
            provider->LoadParameters(&result.result, {*this, util::fnTag<&LoadParameters::Callback>});
            return true;
        }
        struct Result {
            Status status;
            ITextureDataProvider::TextureParameters result;
        };

        auto await_resume() -> Result
        { return result; }
        void Callback(Status status)
        {
            result.status = status;
            handle();
        }
        ITextureDataProvider* provider;
        std::coroutine_handle<> handle;
        Result result;
    };
    struct LoadData {
        bool await_ready() { return false; }
        bool await_suspend(std::coroutine_handle<> handle)
        {
            this->handle = handle;
            provider->LoadData(data, 0, {*this, util::fnTag<&LoadData::Callback>});
            return true;
        }
        auto await_resume() -> Status
        { return status; }
        void Callback(Status status)
        {
            this->status = status;
            handle();
        }
        ITextureDataProvider* provider;
        void* data;
        std::coroutine_handle<> handle;
        Status status;
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
    auto coro = +[](ITextureDataProvider *provider) -> util::auto_task<Image&&> {
        Image image;
        auto [status, params] = co_await LoadParameters{ provider };
        if (IsError(status)) {
            co_return image;
        }
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
            co_return image;
        }
        image.size = { params.width, params.height };
        image.data.reset(::operator new(params.height * params.width * ImageManipulator::PixelSize));
        status = co_await LoadData{ provider, image.data.get() };
        co_return image;
    };
    coro(provider).start(callback);
}

} // namespace dse::core
