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
    {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_RGBA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_RGB, GL_UNSIGNED_BYTE, 3},
    {GL_SRGB8_ALPHA8, GL_BGRA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_BGRA, GL_UNSIGNED_BYTE, 4},
    {GL_SRGB, GL_BGR, GL_UNSIGNED_BYTE, 3},
    {GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, 4},
};

}

TextureInstance::TextureInstance() :
    TextureInstance(nullptr)
{

}

TextureInstance::TextureInstance(core::ITextureDataProvider* texture) :
    textureProvider(texture),
    texture(false),
    lastVersion(0),
    readyStatus(Ready)
{}

bool TextureInstance::IsReady()
{
    switch (readyStatus.load(std::memory_order_acquire)) {
        case Ready: {
            if (texture && lastVersion == textureProvider->GetVersion()) {
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

void TextureInstance::BeginLoad()
{
    readyStatus.store(Pending, std::memory_order_release);
    textureProvider->LoadParameters(
        &textureParameters,
        {*this, util::fnTag<&TextureInstance::LoadTexture>}
    );
}

void TextureInstance::LoadTexture()
{
    GLFormatMapEntry* format;
    if (textureParameters.format < std::size(glFormatMap)) {
        format = glFormatMap + textureParameters.format;
    } else {
        format = glFormatMap + 1;
    }
    std::size_t size = std::size_t((std::abs(textureParameters.width) * format->pixelSize + 3) & (~3)) *
        std::abs(textureParameters.height) * std::abs(textureParameters.depth);
    textureData.resize(size);
    textureProvider->LoadData(textureData.data(), 0, {*this, util::fnTag<&TextureInstance::TextureReady>});
}

void TextureInstance::TextureReady()
{
    readyStatus.store(UploadReady, std::memory_order_release);
}

void TextureInstance::UploadTexture()
{
    GLFormatMapEntry* format;
    if (textureParameters.format < std::size(glFormatMap)) {
        format = glFormatMap + textureParameters.format;
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
    lastVersion = textureProvider->GetVersion();
    readyStatus.store(Ready, std::memory_order_release);
}

void TextureInstance::Deleter::operator()(TextureInstance* inst) const
{
    inst->Release();
}

} // namespace dse::ogl31rbe::gl31
