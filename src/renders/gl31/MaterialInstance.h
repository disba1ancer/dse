#ifndef DSE_RENDERS_GL31_MATERIALINSTANCE_H
#define DSE_RENDERS_GL31_MATERIALINSTANCE_H

#include "RefCounted.h"
#include "scn/Material.h"
#include "renders/glwrp/Buffer.h"
#include "binds.h"
#include "TextureInstance.h"

namespace dse::renders {
class RenderOpenGL31_impl;
}

namespace dse::renders::gl31 {

class MaterialInstance : public RefCounted {
    scn::Material* material;
    std::uint32_t lastVersion;
    glwrp::UniformBuffer ubo;
    std::unique_ptr<TextureInstance, TextureInstance::Deleter> diffuseInstance;
public:
    MaterialInstance(scn::Material* material);
    MaterialInstance(RenderOpenGL31_impl* render, scn::Material* material);
    MaterialInstance();
    void Reload(RenderOpenGL31_impl* render);
    void CheckAndSync(RenderOpenGL31_impl* render);
    bool IsInstanceOf(scn::Material* material) const;
    auto GetUBO() -> glwrp::UniformBuffer&;
    auto GetDiffuseTextureInstance(RenderOpenGL31_impl* render) -> TextureInstance*;
    struct Deleter {
        void operator()(MaterialInstance* inst) const;
    };
};

} // namespace gl31::renders::dse

#endif // DSE_RENDERS_GL31_MATERIALINSTANCE_H
