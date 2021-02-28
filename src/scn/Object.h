/*
 * Object.h
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_OBJECT_H_
#define SCN_OBJECT_H_

#include "math/vec.h"
#include <vector>
#include <map>

namespace dse {
namespace scn {

class IMesh;
class Material;

class Object {
	math::vec3 pos;
	unsigned version = 0;
	math::vec4 qRot;
	math::vec3 scale;
	IMesh* mesh;
	std::vector<Material*> materials;
public:
	Object(
		IMesh* mesh = nullptr,
		math::vec3 pos = {0, 0, 0},
		math::vec4 qRot = {0, 0, 0, 1},
		math::vec3 scale = {1, 1, 1}
	);
	IMesh* getMesh() const;
	void setMesh(IMesh* mesh = nullptr);
	math::vec3 getPos() const;
	void setPos(const math::vec3 &pos);
	math::vec4 getQRot() const;
	void setQRot(const math::vec4 &rot);
	math::vec3 getScale() const;
	void setScale(const math::vec3 &scale);
	unsigned getVersion() const;
	void setMaterial(unsigned materialSlot, Material* mat);
	Material* getMaterial(unsigned materialSlot);
	void storeCustomValue(void *owner, void *value);
	void * getCustomValue(void *owner);
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_OBJECT_H_ */
