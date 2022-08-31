/*
 * ObjectInstance.h
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_
#define SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_

#include <memory>
#include <dse/core/Object.h>
#include "MeshInstance.h"
#include "MaterialInstance.h"
#include <cstdint>

namespace dse::ogl31rbe {
class RenderOpenGL31_impl;
}

namespace dse::ogl31rbe::gl31 {

class ObjectInstance {
	core::Object* object;
	std::unique_ptr<MeshInstance, MeshInstance::Deleter> mesh;
	glwrp::UniformBuffer ubo;
	std::uint32_t lastVersion;
	std::unordered_map<unsigned, std::unique_ptr<MaterialInstance, MaterialInstance::Deleter>> materials;
public:
	ObjectInstance();
	ObjectInstance(core::Object* object);
	ObjectInstance(RenderOpenGL31_impl* render, core::Object* object);
	void Reload(RenderOpenGL31_impl* render);
	void CheckAndSync(RenderOpenGL31_impl* render);
	auto GetMeshInstance() const -> MeshInstance*;
	auto GetMaterialInstance(RenderOpenGL31_impl* render, unsigned index) -> MaterialInstance*;
	auto GetUBO() -> glwrp::UniformBuffer&;
};

} /* namespace dse::ogl31rbe::gl31 */

#endif /* SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_ */
