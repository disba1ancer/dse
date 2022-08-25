/*
 * Scene.h
 *
 *  Created on: 16 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_SCENE_H_
#define DSE_CORE_SCENE_H_

#include "Object.h"
#include <list>
#include <dse/util/ProxyIterator.h>
#include <dse/util/ProxyContainer.h>
#include <dse/notifier/notifier.h>

namespace dse::core {

enum class SceneChangeEventType {
	ObjectCreate,
	ObjectDestroy
};

class Scene {
	std::list<Object> m_objects;
public:
	typedef void(ChangeEvent)(SceneChangeEventType, Object*);
private:
	notifier::notifier<ChangeEvent> changeSubscribers;
public:
	typedef util::ProxyIterator<decltype(m_objects), Scene> ObjectsIterator;
	Scene();
	ObjectsIterator objectsBegin();
	ObjectsIterator objectsEnd();
	typedef util::ProxyContainer<&Scene::objectsBegin, &Scene::objectsEnd> Objects;
	Objects objects();
	ObjectsIterator createObject(Object &&object = Object());
	ObjectsIterator createObject(const Object &object);
	void destroyObject(ObjectsIterator it);
	notifier::connection<ChangeEvent> subscribeChangeEvent(std::function<ChangeEvent> &&callback);
};

} /* namespace dse::core */

#endif /* DSE_CORE_SCENE_H_ */
