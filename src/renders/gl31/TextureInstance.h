#ifndef DSE_RENDERS_GL31_TEXTUREINSTANCE_H
#define DSE_RENDERS_GL31_TEXTUREINSTANCE_H

#include "scn/ITextureDataProvider.h"
#include "RefCounted.h"
#include "renders/glwrp/Texture.h"
#include <atomic>

namespace dse::renders::gl31 {

class TextureInstance : public RefCounted
{
public:
    TextureInstance();
    TextureInstance(scn::ITextureDataProvider* texture);
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
    scn::ITextureDataProvider* textureProvider;
    glwrp::Texture2D texture;
    unsigned lastVersion;
    std::atomic_int readyStatus;
    scn::ITextureDataProvider::TextureParameters textureParameters;
    std::vector<unsigned char> textureData;
};

} // namespace dse::renders::gl31

#endif // DSE_RENDERS_GL31_TEXTUREINSTANCE_H
