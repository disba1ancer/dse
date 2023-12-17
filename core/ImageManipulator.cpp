#include <dse/util/scope_exit.h>
#include <dse/math/mat.h>
#include <dse/core/ImageManipulator.h>
#include <dse/math/vmath.h>
#include <dse/core/Image.h>
#include <cstring>

namespace {

using dse::util::fnTag;
using dse::math::ivec2;
using dse::math::ivec4;
using dse::math::vec4;
using dse::math::mat4;

}

namespace dse::core {

ImageManipulator::ImageManipulator(void *pixels, ivec2 size, bool linear) :
    pixels(static_cast<unsigned char*>(pixels)),
    size(size),
    linear(linear),
    clipBegin({0, 0}),
    clipEnd(size),
    earlyBlendFunc(nullptr),
    blendFunc(nullptr)
{}

ImageManipulator::ImageManipulator(Image &image) :
    ImageManipulator(image.Data(), image.Size(), false)
{
}

auto ImageManipulator::operator[](std::size_t line) -> ImageManipulatorLine
{
    return {pixels + line * size.x() * PixelSize, linear};
}

auto ImageManipulator::operator[](ivec2 pos) -> ImageManipulatorPixel
{
    return (*this)[pos.y()][pos.x()];
}

auto ImageManipulator::Size() -> ivec2
{
    return size;
}

void ImageManipulator::Fill(ivec2 begin, ivec2 size, vec4 color)
{
    Fill(begin, size, ToRawColor(color));
}

void ImageManipulator::Fill(ivec2 begin, ivec2 fsize, std::uint32_t color)
{
    auto cb = clamp(begin, 0, size);
    auto ce = clamp(begin + fsize, 0, size);
    for (int iy = cb.y(); iy < ce.y(); ++iy) {
        for (int ix = cb.x(); ix < ce.x(); ++ix) {
            (*this)[iy][ix] = color;
        }
    }
}

void ImageManipulator::BlitImage(
        ivec2 dstPos, ivec2 size, Image &srcImage, ivec2 srcPos)
{
    auto src = srcImage.Manipulator();
    auto dstBeg = clamp(dstPos, 0, Size());
    auto srcBeg = clamp(srcPos, 0, src.Size());
    auto dstEnd = clamp(dstPos + size, 0, Size());
    auto srcEnd = clamp(srcPos + size, 0, src.Size());
    auto size2 = min(dstEnd - dstBeg, srcEnd - srcBeg);
    for (int y = 0; y < size2.y(); ++y) {
        for (int x = 0; x < size2.x(); ++x) {
            std::uint32_t color = src[srcBeg.y() + y][srcBeg.x() + x];
            (*this)[dstBeg.y() + y][dstBeg.x() + x] = color;
        }
    }
}

ImageManipulator::EarlyBlendFunc ImageManipulator::SetEarlyBlendFunc(EarlyBlendFunc blendFunc)
{
    return std::exchange(earlyBlendFunc, blendFunc);
}

auto ImageManipulator::SetBlendFunc(BlendFunc blendFunc) -> ImageManipulator::BlendFunc
{
    return std::exchange(this->blendFunc, blendFunc);
}

void ImageManipulator::SetClipRect(ivec2 begin, ivec2 cSize)
{
    clipBegin = clamp(begin, 0, size);
    clipEnd = clamp(begin + cSize, clipBegin, size);
}

void ImageManipulator::BlendImage(
        ivec2 dstPos, ivec2 dstSize, Image &srcImage, ivec2 srcPos,
        ivec2 srcSize)
{
    if (dstSize == srcSize) {
        return BlendImage(dstPos, dstSize, srcImage, srcPos);
    }
    auto src = srcImage.Manipulator();
    auto dstBeg = clamp(dstPos, clipBegin, clipEnd);
    auto dstEnd = clamp(dstPos + dstSize, clipBegin, clipEnd);
    auto srcBeg = clamp(srcPos, 0, src.size);
    auto srcEnd = clamp(srcPos + srcSize, 0, src.size);
    auto srcSize2 = srcEnd - srcBeg;
    if (srcSize2.x() == 0 || srcSize2.y() == 0) {
        return;
    }
    for (int cj = dstBeg.y(); cj < dstEnd.y(); ++cj) {
        auto coordY = ((cj - dstPos.y()) * 1024 + 512) * srcSize2.y() / dstSize.y() + 512;
        auto y1 = coordY / 1024 - 1;
        y1 += srcBeg.y() + srcSize2.y() * (y1 < 0);
        auto y2 = coordY / 1024;
        y2 += srcBeg.y() - srcSize2.y() * (y2 == srcSize2.y());
        for (int ci = dstBeg.x(); ci < dstEnd.x(); ++ci) {
            auto pos = ivec2{ci, cj};
            std::uint32_t dstColor = (*this)[pos];
            if (BlendEarly(dstColor)) {
                continue;
            }
            auto coordX = ((ci - dstPos.x()) * 1024 + 512) * srcSize2.x() / dstSize.x() + 512;
            auto x1 = coordX / 1024 - 1;
            x1 += srcBeg.x() + srcSize2.x() * (x1 < 0);
            auto x2 = coordX / 1024;
            x2 += srcBeg.x() - srcSize2.x() * (x2 == srcSize2.x());
            mat4 colors = {
                src[{x1, y1}],
                src[{x2, y1}],
                src[{x1, y2}],
                src[{x2, y2}]
            };
            auto lX = (coordX & (1024 - 1)) / 1024.f;
            auto lY = (coordY & (1024 - 1)) / 1024.f;
            vec4 coefs = {
                (1 - lY) * (1 - lX),
                (1 - lY) * lX,
                lY * (1 - lX),
                lY * lX
            };
            auto color = colors * coefs;
            (*this)[pos] = BlendColor(dstColor, color);
        }
    }
}

void ImageManipulator::DrawPixel(ivec2 pos, vec4 color)
{
    auto pixelRef = (*this)[pos];
    std::uint32_t dstColor = pixelRef;
    if (BlendEarly(dstColor)) {
        return;
    }
    pixelRef = BlendColor(dstColor, color);
}

void ImageManipulator::DrawHLine(ivec2 startPos, int endX, vec4 color)
{
    for (int ci = startPos.x(); ci < endX; ++ci) {
        DrawPixel({ci, startPos.y()}, color);
    }
}

void ImageManipulator::DrawVLine(ivec2 startPos, int endY, vec4 color)
{
    for (int cj = startPos.y(); cj < endY; ++cj) {
        DrawPixel({startPos.x(), cj}, color);
    }
}

void ImageManipulator::DrawRectFilled(ivec2 pos, ivec2 rSize, vec4 color)
{
    auto posBeg = clamp(pos, clipBegin, clipEnd);
    auto posEnd = clamp(pos + rSize, clipBegin, clipEnd);
    for (int cj = posBeg.y(); cj < posEnd.y(); ++cj) {
        DrawHLine({posBeg.x(), cj}, posEnd.x(), color);
    }
}

void ImageManipulator::DrawRect(ivec2 pos, ivec2 size, int thickness, vec4 color)
{
    auto dThick = thickness * 2;
    if (size.x() <= dThick || size.y() <= dThick) {
        DrawRectFilled(pos, size, color);
    }
    auto pos2 = pos + size - thickness;
    DrawRectFilled(pos, {size.x(), thickness}, color);
    DrawRectFilled({pos2.x(), pos.y() + thickness}, {thickness, size.y() - dThick}, color);
    DrawRectFilled({pos.x(), pos.y() + thickness}, {thickness, size.y() - dThick}, color);
    DrawRectFilled({pos.x(), pos2.y()}, {size.x(), thickness}, color);
}

void ImageManipulator::DrawText(ivec2 pos, ivec2 size, char8_t *text, Image &font, ivec2 gSize)
{
    auto oldClipBegin = clipBegin;
    auto oldClipEnd = clipEnd;
    SetClipRect(pos, size);
    util::scope_exit final([this, &oldClipBegin, &oldClipEnd]{
        clipBegin = oldClipBegin;
        clipEnd = oldClipEnd;
    });
    auto cPos = pos;
    auto end = pos + size;
    int gVCount = font.Size().x() / gSize.x();
    char8_t *cur = text;
    while (*cur != 0 && cPos.y() < end.y()) {
        if (*cur == u8'\n') {
            cPos = {pos.x(), cPos.y() + gSize.y()};
            ++cur;
            continue;
        }
        ivec2 gPos = {*cur % gVCount, *cur / gVCount};
        BlendImage(cPos, gSize, font, gPos * gSize);
        cPos.x() += gSize.x();
        ++cur;
        if (cPos.x() < end.x()) {
            continue;
        }
        cur = reinterpret_cast<char8_t*>(std::strchr(reinterpret_cast<char*>(cur), u8'\n'));
        if (cur == nullptr) {
            break;
        }
    }
}

void ImageManipulator::DrawLine(ivec2 pos1, ivec2 pos2, vec4 color)
{
    using std::min;
    using std::max;
    using std::clamp;
    if (pos1.x() == pos2.x()) {
        if (clipBegin.x() > pos1.x() || pos1.x() >= clipEnd.x()) {
            return;
        }
        auto start = max(min(pos1, pos2), clipBegin);
        auto end = min(max(pos2.y(), pos1.y()) + 1, clipEnd.y());
        return DrawVLine(start, end, color);
    }
    if (pos1.y() == pos2.y()) {
        if (clipBegin.y() > pos1.y() || pos1.y() >= clipEnd.y()) {
            return;
        }
        auto start = max(min(pos1, pos2), clipBegin);
        auto end = min(max(pos2.x(), pos1.x()) + 1, clipEnd.x());
        return DrawHLine(start, end, color);
    }
    DrawLine1(pos1, pos2, color);
}

void ImageManipulator::DrawLine1(ivec2 pos1, ivec2 pos2, vec4 color)
{
    if (pos2.x() < pos1.x()) {
        DrawLine2(pos2, pos1, pos1 - pos2, color);
    } else {
        DrawLine2(pos1, pos2, pos2 - pos1, color);
    }
}

void ImageManipulator::DrawLine2(ivec2 pos1, ivec2 pos2, ivec2 dpos, vec4 color)
{
    using std::swap;
    using std::abs;
    using std::max;
    dpos.y() = abs(dpos.y());
    auto stepY = 1 - 2 * (pos2.y() < pos1.y());
    auto begin = clipBegin;
    auto end = clipEnd - 1;
    if (stepY < 0) {
        swap(begin.y(), end.y());
    }
    if (pos1.x() > end.x() || stepY * (pos1.y() - end.y()) > 0 ||
        pos2.x() < begin.x() || stepY * (pos2.y() - begin.y()) < 0)
    {
        return;
    }
    ivec2 start;
    start = begin - pos1;
    start *= dpos["yx"];
    if (start.x() >= start.y() * stepY) {
        start.x() = max(pos1.x(), begin.x());
        start.y() = (start.x() - pos1.x()) * dpos.y() * stepY / dpos.x() + pos1.y();
    } else {
        start.y() = max(pos1.y() * stepY, begin.y() * stepY) * stepY;
        start.x() = (start.y() - pos1.y()) * dpos.x() * stepY / dpos.y() + pos1.x();
    }
    int error = unsigned((start.y() - pos1.y()) * stepY + 1) * dpos.x()
            - unsigned(start.x() - pos1.x() + 1) * dpos.y();

    pos1 = start;
    while (pos1.x() <= end.x() && pos1.y() * stepY <= end.y() * stepY) {
        DrawPixel(pos1, color);
        auto derr = 2 * error;
        if (derr > -dpos.y()) {
            if (pos1.x() == pos2.x()) {
                break;
            }
            pos1.x()++;
            error -= dpos.y();
        }
        if (derr < dpos.x()) {
            if (pos1.y() == pos2.y()) {
                break;
            }
            pos1.y() += stepY;
            error += dpos.x();
        }
    }
}

void ImageManipulator::DrawLineInternal2(ivec2 pos1, ivec2 pos2, vec4 color)
{
    using std::abs;
    using std::max;
    using std::swap;
    auto dpos = pos2 - pos1;
    auto ver = abs(dpos.x()) < abs(dpos.y());
    if (dpos[ver] < 0) {
        pos1 += dpos;
        dpos = -dpos;
    }
    auto clipBegin2 = max(clipBegin - pos1, {0, 0});
    auto clipEnd2 = clipEnd - pos1;
    if (dpos[!ver] < 0) {
        swap(clipBegin2[!ver], clipEnd2[!ver]);
    }
    auto clipBegin3 = clipBegin2 * dpos["yx"];
    auto test = clipBegin3[ver] < clipBegin3[!ver];
    // TODO: try start beyond clip area
    auto startPos = (2 * clipBegin3[test ^ ver] - dpos[!test ^ ver]) / (2 * dpos[test ^ ver]);
    ivec2 cur;
    cur[!ver ^ test] = startPos;
    cur[ver ^ test] = clipBegin2[ver ^ test] - 1;
    auto sign = 1 - 2 * (dpos[!ver] < 0);
    auto limit = min(dpos, clipEnd2 - 1);
    while (true) {
        auto a = long(cur[ver] + 1) * dpos[!ver];
        auto b = long(cur[!ver]) * dpos[ver];
        auto incr = (a - b) * sign * 2 > dpos[ver];
        cur[ver] += 1;
        cur[!ver] += incr * sign;
        if (cur[ver] > limit[ver] || sign * (cur[!ver] - limit[!ver]) > 0) {
            break;
        }
        DrawPixel(pos1 + cur, color);
    }
}

void ImageManipulator::DrawLineInternal(ivec2 pos1, ivec2 pos2, vec4 color)
{
    using std::abs;
    if (pos1 != clamp(pos1, clipBegin, clipEnd - 1) ||
        pos2 != clamp(pos2, clipBegin, clipEnd - 1))
    {
        return;
    }
    auto dpos = pos2 - pos1;
    auto ver = abs(dpos.x()) < abs(dpos.y());
    if (pos2[ver] < pos1[ver]) {
        pos1 += dpos;
        dpos = -dpos;
    }
    auto cur = ivec2{0, 0};
    auto sign = 1 - 2 * (dpos[!ver] < 0);
    while (cur[ver] < dpos[ver]) {
        DrawPixel(pos1 + cur, color);
        auto a = long(cur[ver] + 1) * dpos[!ver];
        auto b = long(cur[!ver]) * dpos[ver];
        auto incr = (a - b) * sign * 2 > dpos[ver];
        cur[ver] += 1;
        cur[!ver] += incr * sign;
    }
    DrawPixel(pos1 + cur, color);
}

auto ImageManipulator::ToRawColor(vec4 color) -> std::uint32_t
{
    return ToRawColor(color, linear);
}

auto ImageManipulator::FromRawColor(uint32_t color) -> vec4
{
    return FromRawColor(color, linear);
}

namespace {

auto Identity1(const ivec4& color) -> vec4
{
    return color / 255.f;
}
auto Identity2(const vec4& color) -> ivec4
{
    return color * 255;
}
const struct GammaMapper {
    constexpr GammaMapper() {
        for (int i = 0; i < std::size(linear); ++i) {
            linear[i] = math::ToLinearColor(i / 255.f);
        }
        for (int i = 0; i < 1008; ++i) {
            sRGB[i] = math::ToSRGBColor(i / 4095.f) * 255;
        }
        for (int i = 0; i < 193; ++i) {
            sRGB[1008 + i] = math::ToSRGBColor((63 + i) / 255.f) * 255;
        }
    }
    float linear[256];
    unsigned char sRGB[1201];
} gammaMap;
auto ToLinear(const ivec4& color) -> vec4
{
    return {
        gammaMap.linear[color.x()],
        gammaMap.linear[color.y()],
        gammaMap.linear[color.z()],
        color.w() / 255.f
    };
}
int ToSRGB(float val)
{
    int val2 = val * 4095;
    if (val2 < 1008) {
        return gammaMap.sRGB[val2];
    }
    return gammaMap.sRGB[1008 + (val2 >> 4) - 63];
}
auto ToSRGB(const vec4& color) -> ivec4
{
    return {
        ToSRGB(color.x()),
        ToSRGB(color.y()),
        ToSRGB(color.z()),
        int(color.w() * 255)
    };
}
using TransferFunc = vec4(const vec4& color);
using ToLinearFunc = vec4(const ivec4& color);
using ToSRGBFunc = ivec4(const vec4& color);
constexpr ToLinearFunc* toLinear[2] = {
    ToLinear,
    Identity1
};
constexpr ToSRGBFunc* toSRGB[2] = {
    ToSRGB,
    Identity2
};

}

auto ImageManipulator::ToRawColor(vec4 color, bool linear) -> std::uint32_t
{
    auto color2 = toSRGB[linear](clamp(color, 0.f, 1.f));
    std::uint32_t result = 0;
    result |= std::uint32_t(color2.w()) << 24;
    result |= std::uint32_t(color2.x()) << 16;
    result |= std::uint32_t(color2.y()) << 8;
    result |= std::uint32_t(color2.z());
    return result;
}

auto ImageManipulator::FromRawColor(uint32_t color, bool linear) -> vec4
{
    ivec4 c2 = {
        int((color >> 16) & 0xFF),
        int((color >>  8) & 0xFF),
        int( color        & 0xFF),
        int((color >> 24) & 0xFF)
    };
    auto result = toLinear[linear](c2);
    return result;
}

void ImageManipulator::BlendImage(ivec2 dstPos, ivec2 imgSize,
                                  Image &srcImage, ivec2 srcPos)
{
    auto src = srcImage.Manipulator();
    auto dstBeg = clamp(dstPos, clipBegin, clipEnd);
    auto dstEnd = clamp(dstPos + imgSize, clipBegin, clipEnd);
    auto srcBeg = clamp(srcPos, 0, src.size);
    auto srcEnd = clamp(srcPos + imgSize, 0, src.size);
    dstEnd = min(dstEnd, dstBeg + srcEnd - srcBeg);
    for (int cj = dstBeg.y(); cj < dstEnd.y(); ++cj) {
        for (int ci = dstBeg.x(); ci < dstEnd.x(); ++ci) {
            ivec2 pos = {ci, cj};
            std::uint32_t dstColor = (*this)[pos];
            if (BlendEarly(dstColor)) {
                continue;
            }
            vec4 color = src[srcBeg + pos - dstPos];
            (*this)[pos] = BlendColor(dstColor, color);
        }
    }
}

auto ImageManipulator::BlendColor(std::uint32_t dstColor, vec4 srcColor)
    -> std::uint32_t
{
    auto func = blendFunc;
    if (!func) {
        return DefaultBlendFunc(dstColor, srcColor);
    }
    return func(dstColor, srcColor);
}

bool ImageManipulator::BlendEarly(uint32_t dstColor)
{
    return earlyBlendFunc && earlyBlendFunc(dstColor);
}

auto ImageManipulator::DefaultBlendFunc(std::uint32_t dstColorR, vec4 srcColorIn)
    -> std::uint32_t
{
    auto alpha = srcColorIn.w();
    switch (int(alpha * 255)) {
    case 0:
        return dstColorR;
    case 255:
        return ToRawColor(srcColorIn);
    default:
        break;
    }
    auto srcColor = srcColorIn * alpha;
    auto dstColor = ImageManipulator::FromRawColor(dstColorR) * (1 - alpha);
    return ToRawColor(dstColor + srcColor);
}

} // namespace dse::core
