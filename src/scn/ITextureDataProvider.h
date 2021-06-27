#ifndef ITEXTUREDATAPROVIDER_H
#define ITEXTUREDATAPROVIDER_H

struct ITextureDataProvider {
	enum PixelFormat {
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

	virtual int getWidth() = 0;
	virtual int getHeight() = 0;
	virtual int getDepth() = 0;
	virtual PixelFormat getFormat() = 0;
	virtual unsigned getLodCount() = 0;
	virtual void uploadData(void* recvBuffer, unsigned lod) = 0;
	~ITextureDataProvider() = default;
};

#endif // ITEXTUREDATAPROVIDER_H
