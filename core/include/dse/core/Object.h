/*
 * Object.h
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_OBJECT_H_
#define DSE_CORE_OBJECT_H_

#include <dse/math/vec.h>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace dse::core {

class IMesh;
class Material;

class Object {
	math::vec3 pos;
	std::uint32_t version;
	math::vec4 qRot;
	math::vec3 scale;
	IMesh* mesh;
	std::unordered_map<unsigned, Material*> materials;
public:
	Object(
		IMesh* mesh = nullptr,
		math::vec3 pos = {0, 0, 0},
		math::vec4 qRot = {0, 0, 0, 1},
		math::vec3 scale = {1, 1, 1}
	);
	auto GetMesh() const -> IMesh*;
	void SetMesh(IMesh* mesh = nullptr);
	auto GetPos() const -> math::vec3;
	void SetPos(const math::vec3 &pos);
	auto GetQRot() const -> math::vec4;
	void SetQRot(const math::vec4 &rot);
	auto GetScale() const -> math::vec3;
	void SetScale(const math::vec3 &scale);
	auto GetVersion() const -> std::uint32_t;
	void SetMaterial(unsigned materialSlot, Material* mat);
	auto GetMaterial(unsigned materialSlot) -> Material*;
	void StoreCustomValue(void *owner, void *value);
	auto GetCustomValue(void *owner) -> void*;
private:
	void IncrementVersion();
};

} /* namespace dse::core */

#endif /* DSE_CORE_OBJECT_H_ */
