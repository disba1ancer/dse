#ifndef ITEXTUREDATAPROVIDER_H
#define ITEXTUREDATAPROVIDER_H

#include <util/functional.h>

namespace dse::scn {

struct ITextureDataProvider {
	enum PixelFormat {
		Unsupported,
		RGBA8,
		RGB8,
		BGRA8,
		BGR8,
		RGBA8sRGB,
		RGB8sRGB,
		BGRA8sRGB,
		BGR8sRGB,
		R11G11B10
	};

	struct PixelFormatParameters {
		unsigned char pixelSize;
	};
	static const PixelFormatParameters pixelFormatParameters[];
	struct TextureParameters {
		int width;
		int height;
		int depth;
		PixelFormat format;
		unsigned lodCount;
	};

	virtual void LoadParameters(TextureParameters* parameters, util::FunctionPtr<void()> onReady) = 0;
	virtual void LoadData(void* recvBuffer, unsigned lod, util::FunctionPtr<void()> onReady) = 0;
	virtual auto GetVersion() -> unsigned = 0;
	~ITextureDataProvider() = default;
};

}

#endif // ITEXTUREDATAPROVIDER_H
