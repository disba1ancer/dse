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

BasicBitmapLoader::BasicBitmapLoader(ThreadPool& pool, const char8_t* file) :
    width(0),
    height(0),
    format(PixelFormat::Unsupported),
    redShift(16), greenShift(8), blueShift(0), alphaShift(24), redLength(8), greenLength(8), blueLength(8), alphaLength(8)
{
	if (file == nullptr || file[0] == '\0') {
		return;
	}
	bitmapFile = { pool, file, OpenMode::Read };
}

void BasicBitmapLoader::LoadParameters(TextureParameters* parameters, util::FunctionPtr<void ()> onReady)
{
	util::StartDetached(LoadParametersInternal(parameters, onReady));
}

void BasicBitmapLoader::LoadData(void* recvBuffer, unsigned lod, util::FunctionPtr<void ()> onReady)
{
	util::StartDetached(LoadDataInternal(recvBuffer, lod, onReady));
}

unsigned BasicBitmapLoader::GetVersion()
{
	return 1;
}

util::Task<void> BasicBitmapLoader::LoadParametersInternal(TextureParameters* parameters, util::FunctionPtr<void ()> onReady)
{
	if (format == Unsupported) {
		std::uint_least16_t sign;
		co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&sign), sizeof(sign));
		if ((sign & 0xFFFF) != 0x4D42) {
			throw std::runtime_error("Bitmap signature error");
		}
		BitmapMeta bitmapMeta;
		co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&bitmapMeta), sizeof(bitmapMeta));
		if (bitmapMeta.info.height < 0) {
			throw std::runtime_error("Negative height not supported");
		}
		if (bitmapMeta.info.compression != BMCOMPR_RGB && bitmapMeta.info.compression != BMCOMPR_BITFIELDS) {
			throw std::runtime_error("Compression in not supported");
		}
		if (bitmapMeta.info.bpp == 24) {
			format = BGR8sRGB;
		} else if (bitmapMeta.info.bpp == 32) {
			format = RGBA8sRGB;
			if (bitmapMeta.info.compression == BMCOMPR_BITFIELDS) {
				ColorMasks clMasks;
				co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&clMasks), sizeof(clMasks));
				maskToShift(clMasks.redMask, redShift, redLength);
				maskToShift(clMasks.greenMask, greenShift, greenLength);
				maskToShift(clMasks.blueMask, blueShift, blueLength);
				maskToShift(clMasks.alphaMask, alphaShift, alphaLength);
			}
		} else {
			throw std::runtime_error("Only 24 and 32 BPP supported");
		}
		width = bitmapMeta.info.width;
		height = bitmapMeta.info.height;
		pixelsPos = bitmapMeta.header.bitmapDataStart;
	}
	parameters->width = width;
	parameters->height = height;
	parameters->depth = 1;
	parameters->lodCount = 1;
	parameters->format = format;
	onReady();
}

util::Task<void> BasicBitmapLoader::LoadDataInternal(void* recvBuffer, unsigned lod, util::FunctionPtr<void ()> onReady)
{
	bitmapFile.Seek(pixelsPos);
	switch (format) {
	case BGR8sRGB: {
		size_t dataSize = (3 * size_t(width) + 3) & ~3U;
		auto buffer = static_cast<std::byte*>(recvBuffer) + dataSize * height;
		for (int i = height; i > 0; --i) {
			buffer -= dataSize;
			co_await bitmapFile.ReadAsync(buffer, dataSize);
		}
	} break;
	case RGBA8sRGB: {
		size_t dataSize = 4 * size_t(width);
		auto buffer = static_cast<std::byte*>(recvBuffer) + dataSize * height;
		for (int i = height; i > 0; --i) {
			uint32_t pixel;
			buffer -= dataSize;
			auto linePtr = reinterpret_cast<unsigned char*>(buffer);
			for (int j = 0; j < width; ++j, linePtr += 4) {
				co_await bitmapFile.ReadAsync(reinterpret_cast<std::byte*>(&pixel), 4);
				linePtr[3] = applyShiftLength(pixel, alphaShift, alphaLength, 0);
				linePtr[2] = applyShiftLength(pixel, blueShift, blueLength, 0);
				linePtr[1] = applyShiftLength(pixel, greenShift, greenLength, 0);
				linePtr[0] = applyShiftLength(pixel, redShift, redLength, 0);
			}
		}
	} break;
	default:
		throw std::runtime_error("Internal error: invalid format");
	}
	onReady();
}

} // namespace dse::scn
