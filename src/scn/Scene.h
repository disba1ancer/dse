/*
 * Scene.h
 *
 *  Created on: 16 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_SCENE_H_
#define SCN_SCENE_H_

#include "Object.h"
#include <list>
#include "../util/ProxyIterator.h"
#include "../util/ProxyContainer.h"
#include "../notifier/notifier.h"

namespace dse {
namespace scn {

enum class SceneChangeEventType {
	OBJECT_CREATE,
	OBJECT_DESTROY
};

class Scene {
	std::list<Object> m_objects;
	typedef void(ChangeEvent)(SceneChangeEventType, Object*);
	notifier::notifier<ChangeEvent> changeSubscribers;
public:
	typedef util::ProxyIterator<decltype(m_objects), Scene> ObjectsIterator;
	Scene();
	~Scene() = default;
	Scene(const Scene &other) = default;
	Scene(Scene &&other) = default;
	Scene& operator=(const Scene &other) = default;
	Scene& operator=(Scene &&other) = default;
	ObjectsIterator objectsBegin();
	ObjectsIterator objectsEnd();
	typedef util::ProxyContainer<&objectsBegin, &objectsEnd> Objects;
	Objects objects();
	ObjectsIterator createObject(Object &&object = Object());
	ObjectsIterator createObject(const Object &object);
	void destroyObject(ObjectsIterator it);
	notifier::connection<ChangeEvent> subscribeChangeEvent(std::function<ChangeEvent> &&callback);
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_SCENE_H_ */
