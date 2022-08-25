#ifndef DSE_RENDERS_GL31_MATERIALINSTANCE_H
#define DSE_RENDERS_GL31_MATERIALINSTANCE_H

#include "RefCounted.h"
#include <dse/core/Material.h>
#include "../glwrp/Buffer.h"
#include "binds.h"
#include "TextureInstance.h"

namespace dse::ogl31rbe {
class RenderOpenGL31_impl;
}

namespace dse::ogl31rbe::gl31 {

class MaterialInstance : public RefCounted {
    core::Material* material;
    std::uint32_t lastVersion;
    glwrp::UniformBuffer ubo;
    std::unique_ptr<TextureInstance, TextureInstance::Deleter> diffuseInstance;
public:
    MaterialInstance(core::Material* material);
    MaterialInstance(RenderOpenGL31_impl* render, core::Material* material);
    MaterialInstance();
    void Reload(RenderOpenGL31_impl* render);
    void CheckAndSync(RenderOpenGL31_impl* render);
    bool IsInstanceOf(core::Material* material) const;
    auto GetUBO() -> glwrp::UniformBuffer&;
    auto GetDiffuseTextureInstance(RenderOpenGL31_impl* render) -> TextureInstance*;
    struct Deleter {
        void operator()(MaterialInstance* inst) const;
    };
};

} // namespace dse::ogl31rbe::gl31

#endif // DSE_RENDERS_GL31_MATERIALINSTANCE_H
