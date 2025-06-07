#ifndef DSE_CORE_BASICBITMAPLOADER_H
#define DSE_CORE_BASICBITMAPLOADER_H

#include "scene2.h"
#include <dse/util/coroutine.h>
#include <dse/core/CachedFile.h>
#include "detail/impexp.h"

namespace dse::core {

class API_DSE_CORE BasicBitmapLoader : public ITexture
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
    auto LoadParameters(TextureParameters* parameters, util::function_ptr<void (Status)>) -> Status override;
    auto LoadData(void* recvBuffer, unsigned lod, util::function_ptr<void (Status)> onReady) -> Status  override;
    auto GetResourceManager() -> IResourceManager& override;
private:
    auto LoadParametersInternal(TextureParameters* parameters) -> util::auto_task<Status>;
    auto LoadDataInternal(void* recvBuffer, unsigned lod) -> util::auto_task<Status>;
};

} // namespace dse::core

#endif // DSE_CORE_BASICBITMAPLOADER_H
