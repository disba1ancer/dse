/*
 * Scene.cpp
 *
 *  Created on: 16 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "Scene.h"

namespace dse {
namespace scn {

Scene::Scene() {
	// TODO Auto-generated constructor stub

}

Scene::~Scene() {
	// TODO Auto-generated destructor stub
}

Scene::ObjectsIterator Scene::objectsBegin() {
	return ObjectsIterator(m_objects.begin());
}

Scene::ObjectsIterator Scene::objectsEnd() {
	return ObjectsIterator(m_objects.end());
}

Scene::ObjectsIterator Scene::objectsAdd(Object &&object) {
	return ObjectsIterator(m_objects.emplace(m_objects.end(), std::move(object)));
}

Scene::ObjectsIterator Scene::objectsAdd(const Object &object) {
	return ObjectsIterator(m_objects.emplace(m_objects.end(), object));
}

Scene::Objects Scene::objects() {
	return Objects(this);
}

void Scene::objectsRemove(ObjectsIterator it) {
	m_objects.erase(it.it);
}

} /* namespace scn */
} /* namespace dse */
