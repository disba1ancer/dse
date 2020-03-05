/*
 * Object.h
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_OBJECT_H_
#define SCN_OBJECT_H_

#include "../math/vec.h"

namespace dse {
namespace scn {

class IMesh;

class Object {
	math::vec3 pos;
	unsigned version = 0;
	math::vec4 qRot;
	math::vec3 scale;
	IMesh* mesh;
public:
	Object(IMesh* mesh = nullptr, math::vec3 pos = {0, 0, 0}, math::vec4 qRot = {0, 0, 0, 1}, math::vec3 scale = {1, 1, 1});
	~Object() = default;
	Object(const Object &other) = default;
	Object(Object &&other) = default;
	Object& operator=(const Object &other) = default;
	Object& operator=(Object &&other) = default;
	IMesh* getMesh() const;
	void setMesh(IMesh* mesh = nullptr);
	math::vec3 getPos() const;
	void setPos(const math::vec3 &pos);
	math::vec4 getQRot() const;
	void setQRot(const math::vec4 &rot);
	math::vec3 getScale() const;
	void setScale(const math::vec3 &scale);
	unsigned getVersion() const;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_OBJECT_H_ */