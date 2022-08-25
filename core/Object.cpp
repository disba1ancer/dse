/*
 * Object.cpp
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: disba1ancer
 */

#include <dse/core/Object.h>
#include <mutex>
#include <dse/core/IMesh.h>

namespace dse::core {

Object::Object(IMesh* mesh, math::vec3 pos, math::vec4 qRot, math::vec3 scale) :
	pos(pos),
	version(1),
	qRot(qRot),
	scale(scale),
	mesh(mesh)
{}

IMesh* Object::GetMesh() const {
	return mesh;
}

void Object::SetMesh(IMesh* mesh) {
	this->mesh = mesh;
	materials.clear();
	IncrementVersion();
}

math::vec3 Object::GetPos() const {
	return pos;
}

void Object::SetPos(const math::vec3 &pos) {
	this->pos = pos;
	IncrementVersion();
}

math::vec4 Object::GetQRot() const {
	return qRot;
}

void Object::SetQRot(const math::vec4 &rot) {
	qRot = rot;
	IncrementVersion();
}

math::vec3 Object::GetScale() const {
	return scale;
}

void Object::SetScale(const math::vec3 &scale) {
	this->scale = scale;
	IncrementVersion();
}

unsigned Object::GetVersion() const {
	return version;
}

void Object::SetMaterial(unsigned materialSlot, Material *mat) {
	if (mat) {
		materials[materialSlot] = mat;
	} else {
		materials.erase(materialSlot);
	}
	IncrementVersion();
}

Material* Object::GetMaterial(unsigned materialSlot) {
	auto it = materials.find(materialSlot);
	if (it == materials.end()) {
		return nullptr;
	}
	return it->second;
}

void Object::IncrementVersion()
{
	bool t = !++version;
	version += t;
}

} /* namespace dse::core */
