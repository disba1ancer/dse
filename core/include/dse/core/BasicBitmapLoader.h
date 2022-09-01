#ifndef DSE_CORE_BASICBITMAPLOADER_H
#define DSE_CORE_BASICBITMAPLOADER_H

#include <fstream>
#include "ITextureDataProvider.h"
#include <dse/util/coroutine.h>
#include <dse/core/File.h>
#include "detail/impexp.h"

namespace dse::core {

class API_DSE_CORE BasicBitmapLoader : public ITextureDataProvider
{
public:
	BasicBitmapLoader(ThreadPool& pool, const char8_t* file = nullptr);
private:
	File bitmapFile;
	int width, height;
	PixelFormat format;
	File::FilePos pixelsPos;
	unsigned char redShift, greenShift, blueShift, alphaShift, redLength, greenLength, blueLength, alphaLength;

	// ITextureDataProvider interface
public:
	virtual void LoadParameters(TextureParameters* parameters, util::FunctionPtr<void ()>) override;
	virtual void LoadData(void* recvBuffer, unsigned lod, util::FunctionPtr<void ()> onReady) override;
	virtual unsigned GetVersion() override;
private:
	auto LoadParametersInternal(TextureParameters* parameters, util::FunctionPtr<void ()> onReady) -> util::Task<void>;
	auto LoadDataInternal(void* recvBuffer, unsigned lod, util::FunctionPtr<void ()> onReady) -> util::Task<void>;
};

} // namespace dse::core

#endif // DSE_CORE_BASICBITMAPLOADER_H
