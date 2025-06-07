#include "MaterialInstance.h"
#include "../RenderOpenGL31_impl.h"

using namespace gl;

namespace dse::ogl31rbe::gl31 {

MaterialInstance::MaterialInstance(core::IMaterial *material) :
    material(material),
    ubo(false)
{}

MaterialInstance::MaterialInstance(RenderOpenGL31_impl* render, core::IMaterial *material) :
    MaterialInstance(material)
{
    Reload(render);
}

MaterialInstance::MaterialInstance() :
    MaterialInstance(nullptr)
{}

void MaterialInstance::Reload(RenderOpenGL31_impl* render)
{
    ObjectMaterialUniform data;
    data.color = material->GetColor();
    if (ubo == 0) {
        ubo = {};
        glBufferData(ubo.target, sizeof(data), nullptr, GL_DYNAMIC_DRAW);
    }
    ubo.bind();
    glBufferSubData(ubo.target, 0, sizeof(data), &data);
    auto& texture = material->GetDiffuseTexture();
    //if (texture) {
        diffuseInstance.reset(render->GetTextureInstance(&texture, true));
    //}
    auto& normalMap = material->GetNormalMapTexture();
    //if (texture) {
        normalMapInstance.reset(render->GetTextureInstance(&normalMap, true));
    //}
    valid = true;
}

void MaterialInstance::CheckAndSync(RenderOpenGL31_impl* render)
{
    if (ubo == 0 || !valid) {
        Reload(render);
    }
}

bool MaterialInstance::IsInstanceOf(core::IMaterial *material) const
{
    return material == this->material;
}

auto MaterialInstance::GetUBO() -> glwrp::UniformBuffer&
{
    return ubo;
}

auto MaterialInstance::GetDiffuseTextureInstance(RenderOpenGL31_impl* render) -> TextureInstance*
{
    return diffuseInstance.get();
}

auto MaterialInstance::GetNormalmapInstance(RenderOpenGL31_impl *render) -> TextureInstance*
{
    return normalMapInstance.get();
}

void MaterialInstance::Invalidate()
{
    valid = false;
}

void MaterialInstance::Deleter::operator()(MaterialInstance* inst) const
{
    inst->Release();
}

} // namespace dse::ogl31rbe::gl31
