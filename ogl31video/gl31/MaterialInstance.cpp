#include "MaterialInstance.h"
#include "../RenderOpenGL31_impl.h"

using namespace gl;

namespace dse::ogl31rbe::gl31 {

MaterialInstance::MaterialInstance(core::Material* material) :
    material(material),
    lastVersion(0),
    ubo(false)
{}

MaterialInstance::MaterialInstance(RenderOpenGL31_impl* render, core::Material* material) :
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
    auto texture = material->GetTexture();
    if (texture) {
        diffuseInstance.reset(render->GetTextureInstance(texture, true));
    }
    texture = material->GetNormalMap();
    if (texture) {
        normalMapInstance.reset(render->GetTextureInstance(texture, true));
    }
    lastVersion = material->GetVersion();
}

void MaterialInstance::CheckAndSync(RenderOpenGL31_impl* render)
{
    if (ubo == 0 || lastVersion != material->GetVersion()) {
        Reload(render);
    }
}

bool MaterialInstance::IsInstanceOf(core::Material* material) const
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

void MaterialInstance::Deleter::operator()(MaterialInstance* inst) const
{
    inst->Release();
}

} // namespace dse::ogl31rbe::gl31
