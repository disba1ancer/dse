/*
 * Scene.cpp
 *
 *  Created on: 16 янв. 2020 г.
 *      Author: disba1ancer
 */

#include "Scene.h"

namespace dse {
namespace scn {

Scene::Scene() {}

Scene::ObjectsIterator Scene::objectsBegin() {
	return ObjectsIterator(m_objects.begin());
}

Scene::ObjectsIterator Scene::objectsEnd() {
	return ObjectsIterator(m_objects.end());
}

Scene::ObjectsIterator Scene::createObject(Object &&object) {
	ObjectsIterator it(m_objects.emplace(m_objects.end(), std::move(object)));
	changeSubscribers.notify(SceneChangeEventType::ObjectCreate, &(*it));
	return it;
}

Scene::ObjectsIterator Scene::createObject(const Object &object) {
	ObjectsIterator it(m_objects.emplace(m_objects.end(), object));
	changeSubscribers.notify(SceneChangeEventType::ObjectCreate, &(*it));
	return it;
}

Scene::Objects Scene::objects() {
	return Objects(this);
}

void Scene::destroyObject(ObjectsIterator it) {
	changeSubscribers.notify(SceneChangeEventType::ObjectDestroy, &(*it));
	m_objects.erase(it.it);
}

notifier::connection<Scene::ChangeEvent> Scene::subscribeChangeEvent(std::function<ChangeEvent> &&callback) {
	return changeSubscribers.subscribe(std::move(callback));
}

} /* namespace scn */
} /* namespace dse */
