#ifndef DSE_SCN_BASICBITMAPLOADER_H
#define DSE_SCN_BASICBITMAPLOADER_H

#include <fstream>
#include "ITextureDataProvider.h"
#include "util/coroutine.h"
#include "core/File.h"

namespace dse {
namespace scn {

class BasicBitmapLoader : public ITextureDataProvider
{
public:
	BasicBitmapLoader(core::ThreadPool& pool, const char8_t* file = nullptr);
private:
	core::File bitmapFile;
	int width, height;
	PixelFormat format;
	core::File::FilePos pixelsPos;
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

} // namespace scn
} // namespace dse

#endif // DSE_SCN_BASICBITMAPLOADER_H
