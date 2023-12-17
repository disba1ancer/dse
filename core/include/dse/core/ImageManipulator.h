#ifndef DSE_CORE_IMAGEMANIPULATOR_H
#define DSE_CORE_IMAGEMANIPULATOR_H

#include <dse/core/detail/impexp.h>
#include <dse/math/vec.h>
#include <dse/math/color.h>
#include <dse/util/functional.h>
#include <cstdint>

namespace dse::core {

class ImageManipulatorLine;
class ImageManipulatorPixel;
class Image;

class API_DSE_CORE ImageManipulator
{
public:
    static constexpr std::size_t PixelSize = 4;
    ImageManipulator(void* pixels, math::ivec2 size, bool linear = false);
    ImageManipulator(Image& image);
    auto operator[](std::size_t line) -> ImageManipulatorLine;
    auto operator[](math::ivec2 pos) -> ImageManipulatorPixel;
    auto Size() -> math::ivec2;
    void Fill(math::ivec2 begin, math::ivec2 size, math::vec4 color);
    void Fill(math::ivec2 begin, math::ivec2 size, std::uint32_t color);
    void BlitImage(math::ivec2 dstPos, math::ivec2 size, Image& srcImage,
                   math::ivec2 srcPos);
    using EarlyBlendFunc = util::FunctionPtr<bool(std::uint32_t)>;
    auto SetEarlyBlendFunc(EarlyBlendFunc blendFunc) -> EarlyBlendFunc;
    using BlendFunc = util::FunctionPtr<std::uint32_t(std::uint32_t, math::vec4)>;
    auto SetBlendFunc(BlendFunc blendFunc) -> BlendFunc;
    void SetClipRect(math::ivec2 begin, math::ivec2 size);
    void BlendImage(math::ivec2 dstPos, math::ivec2 dstSize, Image& srcImage,
                    math::ivec2 srcPos, math::ivec2 srcSize);
    void BlendImage(math::ivec2 dstPos, math::ivec2 imgSize, Image& srcImage,
                    math::ivec2 srcPos);
    void DrawRectFilled(math::ivec2 pos, math::ivec2 size, math::vec4 color);
    void DrawRect(math::ivec2 pos, math::ivec2 size, int thickness, math::vec4 color);
    void DrawText(math::ivec2 pos, math::ivec2 size, char8_t* text, Image& font, math::ivec2 gCount);
    void DrawLine(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    auto ToRawColor(math::vec4 color) -> std::uint32_t;
    auto FromRawColor(std::uint32_t color) -> math::vec4;
    static auto ToRawColor(math::vec4 color, bool linear) -> std::uint32_t;
    static auto FromRawColor(std::uint32_t color, bool linear) -> math::vec4;
private:
    auto BlendColor(std::uint32_t dstColorR, math::vec4 srcColorIn) -> std::uint32_t;
    auto BlendEarly(std::uint32_t dstColor) -> bool;
    auto DefaultBlendFunc(std::uint32_t dstColorR, math::vec4 srcColorIn)
        -> std::uint32_t;
    void DrawPixel(math::ivec2 pos, math::vec4 color);
    void DrawHLine(math::ivec2 pos, int endX, math::vec4 color);
    void DrawVLine(math::ivec2 pos, int endY, math::vec4 color);
    void DrawLineInternal(math::ivec2 pos1, math::ivec2 dpos, math::vec4 color);
    void DrawLineInternal2(math::ivec2 pos1, math::ivec2 dpos, math::vec4 color);
    void DrawLine1(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    void DrawLine2(math::ivec2 pos1, math::ivec2 pos2, math::ivec2 dpos, math::vec4 color);
    void DrawLine3(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    void DrawLine4(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    void DrawLine5(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    void DrawLine6(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    void DrawLine7(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    void DrawLine8(math::ivec2 pos1, math::ivec2 pos2, math::vec4 color);
    unsigned char* pixels;
    math::ivec2 size;
    bool linear;
    math::ivec2 clipBegin;
    math::ivec2 clipEnd;
    EarlyBlendFunc earlyBlendFunc;
    BlendFunc blendFunc;
};

class ImageManipulatorPixel {
public:
    friend class ImageManipulator;
    ImageManipulatorPixel(void* pixel, bool linear) :
        pixel(static_cast<unsigned char*>(pixel)),
        linear(linear)
    {}
    ImageManipulatorPixel& operator=(std::uint32_t clr)
    {
        for (std::size_t i = 0; i < ImageManipulator::PixelSize; ++i, clr >>= 8) {
            pixel[i] = clr & 0xFF;
        }
        return *this;
    }
    ImageManipulatorPixel& operator=(math::ivec4 color)
    {
        pixel[0] = color[2];
        pixel[1] = color[1];
        pixel[2] = color[0];
        pixel[3] = color[3];
        return *this;
    }
    ImageManipulatorPixel& operator=(math::vec4 color)
    {
        return (*this) = ImageManipulator::ToRawColor(color, linear);
    }
    operator std::uint32_t() const {
        std::uint32_t color = 0;
        for (std::size_t i = 0; i < ImageManipulator::PixelSize; ++i) {
            color |= std::uint32_t(pixel[i]) << (i * 8);
        }
        return color;
    }
    operator math::ivec4() const {
        math::ivec4 color;
        color[0] = pixel[2];
        color[1] = pixel[1];
        color[2] = pixel[0];
        color[3] = pixel[3];
        return color;
    }
    operator math::vec4() const {
        return ImageManipulator::FromRawColor(std::uint32_t(*this), linear);
    }
private:
    unsigned char* pixel;
    bool linear;
};

class ImageManipulatorLine {
public:
    ImageManipulatorLine(void* pixels, bool linear) :
        pixels(static_cast<unsigned char*>(pixels)),
        linear(linear)
    {}
    auto operator[](std::size_t pixel) -> ImageManipulatorPixel
    {
        return {pixels + pixel * ImageManipulator::PixelSize, linear};
    }
private:
    unsigned char* pixels;
    bool linear;
};

} // namespace dse::core

#endif // DSE_CORE_IMAGEMANIPULATOR_H
