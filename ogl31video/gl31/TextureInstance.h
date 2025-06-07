#ifndef DSE_RENDERS_GL31_TEXTUREINSTANCE_H
#define DSE_RENDERS_GL31_TEXTUREINSTANCE_H

#include <dse/core/scene2.h>
#include "RefCounted.h"
#include "../glwrp/Texture.h"
#include <atomic>

namespace dse::ogl31rbe::gl31 {

class TextureInstance : public RefCounted
{
public:
    TextureInstance();
    TextureInstance(core::ITexture* texture);
    bool IsReady();
    auto GetTexture() -> glwrp::Texture2D&;
    void Invalidate();
    struct Deleter {
        void operator()(TextureInstance* inst) const;
    };
private:
    void BeginLoad();
    void LoadTexture(core::Status status);
    void TextureReady(core::Status status);
    void UploadTexture();
    core::ITexture* textureProvider;
    glwrp::Texture2D texture;
    bool valid = false;
    std::atomic_int readyStatus;
    core::ITexture::TextureParameters textureParameters;
    std::vector<unsigned char> textureData;
};

} // namespace dse::ogl31rbe::gl31

#endif // DSE_RENDERS_GL31_TEXTUREINSTANCE_H
