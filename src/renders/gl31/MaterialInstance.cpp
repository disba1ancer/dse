#include "MaterialInstance.h"
#include "renders/RenderOpenGL31_impl.h"

using namespace gl;

namespace dse::renders::gl31 {

MaterialInstance::MaterialInstance(scn::Material* material) :
    material(material),
    lastVersion(0),
    ubo(false)
{}

MaterialInstance::MaterialInstance(RenderOpenGL31_impl* render, scn::Material* material) :
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
    lastVersion = material->GetVersion();
}

void MaterialInstance::CheckAndSync(RenderOpenGL31_impl* render)
{
	if (ubo == 0 || lastVersion != material->GetVersion()) {
		Reload(render);
	}
}

bool MaterialInstance::IsInstanceOf(scn::Material* material) const
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

void MaterialInstance::Deleter::operator()(MaterialInstance* inst) const
{
    inst->Release();
}

} // namespace gl31::renders::dse
