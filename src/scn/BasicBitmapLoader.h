#ifndef DSE_SCN_BASICBITMAPLOADER_H
#define DSE_SCN_BASICBITMAPLOADER_H

#include <fstream>
#include "ITextureDataProvider.h"

namespace dse {
namespace scn {

class BasicBitmapLoader : public ITextureDataProvider
{
public:
	BasicBitmapLoader(const char8_t* file = nullptr);
private:
	std::ifstream bitmapFile;
	int width, height;
	PixelFormat format;
	std::ifstream::pos_type pixelsPos;
	unsigned char redShift, greenShift, blueShift, alphaShift, redLength, greenLength, blueLength, alphaLength;

	// ITextureDataProvider interface
public:
	virtual void LoadParameters(TextureParameters* parameters, util::FunctionPtr<void ()> onReady) override;
	virtual void LoadData(void* recvBuffer, unsigned lod, util::FunctionPtr<void ()> onReady) override;
	virtual unsigned GetVersion() override;
};

} // namespace scn
} // namespace dse

#endif // DSE_SCN_BASICBITMAPLOADER_H
