#ifndef DSE_CORE_BASICBITMAPLOADER_H
#define DSE_CORE_BASICBITMAPLOADER_H

#include "ITextureDataProvider.h"
#include <dse/util/coroutine.h>
#include <dse/core/CachedFile.h>
#include "detail/impexp.h"

namespace dse::core {

class API_DSE_CORE BasicBitmapLoader : public ITextureDataProvider
{
public:
    BasicBitmapLoader(IOContext& ctx, const char8_t* file = nullptr, bool linear = false);
private:
    CachedFile bitmapFile;
    int width, height;
    PixelFormat format;
    File::FilePos pixelsPos;
    bool linear;

    // ITextureDataProvider interface
public:
    virtual auto LoadParameters(TextureParameters* parameters, util::FunctionPtr<void (Status)>) -> Status override;
    virtual auto LoadData(void* recvBuffer, unsigned lod, util::FunctionPtr<void (Status)> onReady) -> Status  override;
    virtual unsigned GetVersion() override;
private:
    auto LoadParametersInternal(TextureParameters* parameters) -> util::auto_task<Status>;
    auto LoadDataInternal(void* recvBuffer, unsigned lod) -> util::auto_task<Status>;
};

} // namespace dse::core

#endif // DSE_CORE_BASICBITMAPLOADER_H
