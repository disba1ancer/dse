#include <dse/core/BasicBitmapLoader.h>
#include <dse/core/ThreadPool.h>

namespace {
    struct BitmapHeader {
        uint_least32_t fileSize;
        uint_least32_t pad;
        uint_least32_t bitmapDataStart;
    };

    struct BitmapInfo {
        uint_least32_t structSize;
        int_least32_t width;
        int_least32_t height;
        int_least16_t planesCount;
        int_least16_t bpp;
        uint_least32_t compression;
        uint_least32_t bitmapDataSize;
        int_least32_t pxPerMeterX;
        int_least32_t pxPerMeterY;
        uint_least32_t palColorCount;
        uint_least32_t palColorReqCount;
    };

    enum BitmapCompression {
        BMCOMPR_RGB = 0,
        BMCOMPR_BITFIELDS = 3,
    };

    struct BitmapMeta {
        BitmapHeader header;
        BitmapInfo info;
    };

    struct ColorMasks {
        uint_least32_t redMask;
        uint_least32_t greenMask;
        uint_least32_t blueMask;
        uint_least32_t alphaMask;
    };

    void maskToShift(uint_least32_t mask, unsigned char& shift, unsigned char& length) {
        if (mask != 0) {
            shift = 0;
            while ((mask & 1) == 0) {
                ++shift;
                mask >>= 1;
            }
            length = 0;
            while ((mask & 1) == 1) {
                ++length;
                mask >>= 1;
            }
            if (length > 8) {
                throw std::runtime_error("Mask length is very long");
            }
        } else {
            throw std::runtime_error("Color mask can not be null");
        }
    }

    uint_least32_t applyShiftLength(uint_least32_t color, unsigned char shift, unsigned char length, unsigned char addShift) {
        color >>= shift + length - 8;
        color &= 0xFF;
        color |= color >> length;
        return color << addShift;
    }
}

namespace dse::core {

BasicBitmapLoader::BasicBitmapLoader(IOContext& ctx, const char8_t* file, bool linear) :
    width(0),
    height(0),
    format(PixelFormat::Unsupported),
    linear(linear)
{
    if (file == nullptr || file[0] == '\0') {
        return;
    }
    bitmapFile = { ctx, file, OpenMode::Read };
}

auto BasicBitmapLoader::LoadParameters(TextureParameters* parameters, util::function_ptr<void (Status)> onReady) -> Status
{
    LoadParametersInternal(parameters).start(onReady);
    return Make(status::Code::PendingOperation);
}

auto BasicBitmapLoader::LoadData(void* recvBuffer, unsigned lod, util::function_ptr<void (Status)> onReady) -> Status
{
    LoadDataInternal(recvBuffer, lod).start(onReady);
    return Make(status::Code::PendingOperation);
}

auto BasicBitmapLoader::GetVersion() -> unsigned
{
    return 1;
}

auto BasicBitmapLoader::LoadParametersInternal(TextureParameters* parameters)
-> util::auto_task<Status>
{
    if (format == Unsupported) {
        std::uint_least16_t sign;
        co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&sign), sizeof(sign));
        if ((sign & 0xFFFF) != 0x4D42) {
            co_return Make(status::Code::Unexpected); // Bitmap signature error
        }
        BitmapMeta bitmapMeta;
        co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&bitmapMeta), sizeof(bitmapMeta));
        /*if (bitmapMeta.info.height < 0) {
            throw std::runtime_error("Negative height not supported");
        }*/
        if (bitmapMeta.info.compression != BMCOMPR_RGB && bitmapMeta.info.compression != BMCOMPR_BITFIELDS) {
            co_return Make(status::Code::Unexpected); // Compression is not supported
        }
        if (bitmapMeta.info.bpp == 24) {
            format = BGR8;
        } else if (bitmapMeta.info.bpp == 32) {
            if (bitmapMeta.info.compression != BMCOMPR_BITFIELDS) {
                format = BGRX8;
            } else {
                ColorMasks clMasks;
                co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&clMasks), sizeof(clMasks));
                if (
                    clMasks.redMask == 0xFF0000 &&
                    clMasks.greenMask == 0xFF00 &&
                    clMasks.blueMask == 0xFF &&
                    clMasks.alphaMask == 0xFF000000
                ) {
                    format = BGRA8;
                } else if (
                    clMasks.redMask == 0xFF0000 &&
                    clMasks.greenMask == 0xFF00 &&
                    clMasks.blueMask == 0xFF &&
                    clMasks.alphaMask == 0
                ) {
                    format = BGRX8;
                } else if (
                    clMasks.redMask == 0xFF &&
                    clMasks.greenMask == 0xFF00 &&
                    clMasks.blueMask == 0xFF0000 &&
                    clMasks.alphaMask == 0xFF000000
                ) {
                    format = RGBA8;
                } else if (
                    clMasks.redMask == 0xFF &&
                    clMasks.greenMask == 0xFF00 &&
                    clMasks.blueMask == 0xFF0000 &&
                    clMasks.alphaMask == 0
                ) {
                    format = RGBX8;
                } else {
                    co_return Make(status::Code::Unexpected); // Unsupported 32 bit format
                }
            }
        } else {
            co_return Make(status::Code::Unexpected); // Only 24 and 32 BPP supported
        }
        format = PixelFormat(format + ToSRGB * !linear);
        width = bitmapMeta.info.width;
        height = bitmapMeta.info.height;
        pixelsPos = bitmapMeta.header.bitmapDataStart;
    }
    parameters->width = width;
    parameters->height = std::abs(height);
    parameters->depth = 1;
    parameters->lodCount = 1;
    parameters->format = format;
    co_return Make(status::Code::Success);
}

auto BasicBitmapLoader::LoadDataInternal(void* recvBuffer, unsigned lod)
-> util::auto_task<Status>
{
    bitmapFile.Seek(pixelsPos);
    int bytePerPix;
    switch (format) {
    case BGR8:
    case BGR8sRGB: bytePerPix = 3; break;
    case BGRA8:
    case BGRX8:
    case RGBA8:
    case RGBX8:
    case BGRA8sRGB:
    case BGRX8sRGB:
    case RGBA8sRGB:
    case RGBX8sRGB: bytePerPix = 4; break;
    default:
        co_return Make(status::Code::Unexpected); // Internal error: invalid format
    }
    size_t dataSize = bytePerPix * size_t(width) + 3;
    dataSize &= (dataSize ^ 3);
    if (height < 0) {
        dataSize *= -height;
        auto buffer = static_cast<std::byte*>(recvBuffer);
        co_await bitmapFile.ReadAsync(buffer, dataSize);
    } else {
        auto buffer = static_cast<std::byte*>(recvBuffer) + dataSize * height;
        for (int i = height; i > 0; --i) {
            buffer -= dataSize;
            co_await bitmapFile.ReadAsync(buffer, dataSize);
        }
    }
    co_return Make(status::Code::Success);
}

} // namespace dse::scn
