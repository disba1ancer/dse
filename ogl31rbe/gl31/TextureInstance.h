#ifndef DSE_RENDERS_GL31_TEXTUREINSTANCE_H
#define DSE_RENDERS_GL31_TEXTUREINSTANCE_H

#include <dse/core/ITextureDataProvider.h>
#include "RefCounted.h"
#include "../glwrp/Texture.h"
#include <atomic>

namespace dse::ogl31rbe::gl31 {

class TextureInstance : public RefCounted
{
public:
    TextureInstance();
    TextureInstance(core::ITextureDataProvider* texture);
    bool IsReady();
    auto GetTexture() -> glwrp::Texture2D&;
    struct Deleter {
        void operator()(TextureInstance* inst) const;
    };
private:
    void BeginLoad();
    void LoadTexture();
    void TextureReady();
    void UploadTexture();
    core::ITextureDataProvider* textureProvider;
    glwrp::Texture2D texture;
    unsigned lastVersion;
    std::atomic_int readyStatus;
    core::ITextureDataProvider::TextureParameters textureParameters;
    std::vector<unsigned char> textureData;
};

} // namespace dse::ogl31rbe::gl31

#endif // DSE_RENDERS_GL31_TEXTUREINSTANCE_H
