#include <glbinding/gl31/gl.h>
#include "TextureInstance.h"

using namespace gl31;

namespace dse::ogl31rbe::gl31 {

namespace {

enum {
    Ready,
    Pending,
    UploadReady
};

struct GLFormatMapEntry {
    ::gl::GLenum internFormat;
    ::gl::GLenum format;
    ::gl::GLenum type;
    std::size_t pixelSize;
} glFormatMap[] = {
    {::gl::GLenum{0}, ::gl::GLenum{0}, ::gl::GLenum{0}, 0},
    {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4},
    {GL_RGB, GL_RGBA, GL_UNSIGNED_BYTE, 4},
    {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 3},
    {GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, 4},
    {GL_RGB, GL_BGRA, GL_UNSIGNED_BYTE, 4},
    {GL_RGB, GL_BGR, GL_UNSIGNED_BYTE, 3},
    {GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_RGBA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_RGB, GL_UNSIGNED_BYTE, 3},
    {GL_SRGB_ALPHA, GL_BGRA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_BGRA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_BGR, GL_UNSIGNED_BYTE, 3},
    {GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, 4},
};

}

TextureInstance::TextureInstance() :
    TextureInstance(nullptr)
{

}

TextureInstance::TextureInstance(core::ITexture* texture) :
    textureProvider(texture),
    texture(false),
    readyStatus(Ready)
{}

bool TextureInstance::IsReady()
{
    switch (readyStatus.load(std::memory_order_acquire)) {
        case Ready: {
            if (texture && valid) {
                return true;
            }
            BeginLoad();
            if (readyStatus.load(std::memory_order_acquire) == Pending) {
                return false;
            }
        } [[fallthrough]];
        case UploadReady: {
            UploadTexture();
            return readyStatus.load(std::memory_order_acquire) == Ready;
        } break;
    }
    return false;
}

auto TextureInstance::GetTexture() -> glwrp::Texture2D&
{
    return texture;
}

void TextureInstance::Invalidate()
{
    valid = false;
}

void TextureInstance::BeginLoad()
{
    readyStatus.store(Pending, std::memory_order_release);
    util::function_ptr f{*this, util::fn_tag<&TextureInstance::LoadTexture>};
    auto status = textureProvider->LoadParameters(&textureParameters, f);
    if (status != core::status::Code::PendingOperation) {
        f(status);
    }
}

void TextureInstance::LoadTexture(core::Status status)
{
    GLFormatMapEntry* format;
    if (std::to_underlying(textureParameters.format) < std::size(glFormatMap)) {
        format = glFormatMap + std::to_underlying(textureParameters.format);
    } else {
        format = glFormatMap + 1;
    }
    std::size_t size = std::size_t((std::abs(textureParameters.width) * format->pixelSize + 3) & (~3)) *
        std::abs(textureParameters.height) * std::abs(textureParameters.depth);
    textureData.resize(size);
    util::function_ptr f{*this, util::fn_tag<&TextureInstance::TextureReady>};
    status = textureProvider->LoadData(textureData.data(), 0, f);
    if (status != core::status::Code::PendingOperation) {
        f(status);
    }
}

void TextureInstance::TextureReady(core::Status status)
{
    readyStatus.store(UploadReady, std::memory_order_release);
}

void TextureInstance::UploadTexture()
{
    GLFormatMapEntry* format;
    if (std::to_underlying(textureParameters.format) < std::size(glFormatMap)) {
        format = glFormatMap + std::to_underlying(textureParameters.format);
    } else {
        format = glFormatMap + 1;
    }
    texture = {};
    glTexImage2D(
        texture.target, 0,
        format->internFormat,
        textureParameters.width,
        textureParameters.height, 0,
        format->format,
        format->type,
        textureData.data()
    );
    glGenerateMipmap(texture.target);
    textureData.clear();
    valid = true;
    readyStatus.store(Ready, std::memory_order_release);
}

void TextureInstance::Deleter::operator()(TextureInstance* inst) const
{
    inst->Release();
}

} // namespace dse::ogl31rbe::gl31
