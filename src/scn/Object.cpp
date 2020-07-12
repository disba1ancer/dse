/*
 * Object.cpp
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "Object.h"
#include <mutex>
#include "IMesh.h"

namespace dse {
namespace scn {

Object::Object(IMesh* mesh, math::vec3 pos, math::vec4 qRot, math::vec3 scale) :
	pos(pos),
	qRot(qRot),
	scale(scale),
	mesh(mesh)
{}

IMesh* Object::getMesh() const {
	return mesh;
}

void Object::setMesh(IMesh* mesh) {
	this->mesh = mesh;
	++this->version;
}

math::vec3 Object::getPos() const {
	return pos;
}

void Object::setPos(const math::vec3 &pos) {
	this->pos = pos;
	++this->version;
}

math::vec4 Object::getQRot() const {
	return qRot;
}

void Object::setQRot(const math::vec4 &rot) {
	qRot = rot;
	++this->version;
}

math::vec3 Object::getScale() const {
	return scale;
}

void Object::setScale(const math::vec3 &scale) {
	this->scale = scale;
	++this->version;
}

unsigned Object::getVersion() const {
	return version;
}

void Object::setMaterial(unsigned materialSlot, Material *mat) {
	if (mesh) {
		if (materials.empty()) {
			materials.resize(mesh->getMeshParameters().submeshCount);
		}
		if (materialSlot < materials.size()) {
			materials[materialSlot] = mat;
			++version;
		}
	}
}

Material* Object::getMaterial(unsigned materialSlot) {
	return (materials.size() > materialSlot ? materials[materialSlot] : nullptr);
}

} /* namespace scn */
} /* namespace dse */
