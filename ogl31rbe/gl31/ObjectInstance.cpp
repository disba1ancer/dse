/*
 * ObjectInstance.cpp
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#include "ObjectInstance.h"
#include "binds.h"
#include <dse/math/qmath.h>
#include <glbinding/gl31/gl.h>
#include "../RenderOpenGL31_impl.h"

using namespace gl;

namespace dse::ogl31rbe::gl31 {

ObjectInstance::ObjectInstance() :
    object(nullptr),
    ubo(false),
    lastVersion(0)
{}

ObjectInstance::ObjectInstance(core::Object* object) :
    object(object),
    ubo(false),
    lastVersion(0)
{}

ObjectInstance::ObjectInstance(RenderOpenGL31_impl* render, core::Object* object) :
    ObjectInstance(object)
{
    Reload(render);
}

void ObjectInstance::Reload(RenderOpenGL31_impl* render)
{
	auto meshInst = mesh.get();
	if (meshInst == nullptr || meshInst->GetMesh() != object->GetMesh()) {
		auto m = object->GetMesh();
		meshInst = render->GetMeshInstance(m, true);
		mesh.reset(meshInst);
		materials.clear();
	}
	ObjectInstanceUniform uniforms;
	auto rotScale = math::transpose(math::matFromQuat(object->GetQRot()));
	auto vPos = object->GetPos();
	auto vScale = object->GetScale();
	rotScale[0] *= vScale;
	rotScale[1] *= vScale;
	rotScale[2] *= vScale;
	uniforms.transform = { math::vec4
		{rotScale[0].x(), rotScale[0].y(), rotScale[0].z(), vPos.x()},
		{rotScale[1].x(), rotScale[1].y(), rotScale[1].z(), vPos.y()},
		{rotScale[2].x(), rotScale[2].y(), rotScale[2].z(), vPos.z()}
	};
	if (ubo == 0) {
		ubo = {};
		glBufferData(ubo.target, sizeof(uniforms), nullptr, GL_DYNAMIC_DRAW);
	}
	ubo.bind();
	glBufferSubData(ubo.target, 0, sizeof(uniforms), &uniforms);
	lastVersion = object->GetVersion();
}

void ObjectInstance::CheckAndSync(RenderOpenGL31_impl* render)
{
	if (lastVersion != object->GetVersion()) {
		Reload(render);
	}
}

auto ObjectInstance::GetMeshInstance() const -> MeshInstance*
{
	return mesh.get();
}

auto ObjectInstance::GetMaterialInstance(RenderOpenGL31_impl* render, unsigned index) -> MaterialInstance*
{
	auto it = materials.find(index);
	if (it == materials.end()) {
		auto matInst = render->GetMaterialInstance(object->GetMaterial(index), true);
		if (matInst == nullptr) {
			return nullptr;
		}
		auto [it2, emplaceResult] = materials.emplace(
			std::piecewise_construct,
			std::tuple(index),
			std::tuple(matInst)
		);
		if (!emplaceResult) {
			throw std::bad_alloc();
		}
		it = it2;
	}
	return it->second.get();
}

auto ObjectInstance::GetUBO() -> glwrp::UniformBuffer&
{
	return ubo;
}

} /* namespace dse::ogl31rbe::gl31 */
