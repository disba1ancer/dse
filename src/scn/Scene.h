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
#include "../math/vec.h"
#include "../util/ProxyIterator.h"
#include "../util/ProxyContainer.h"

namespace dse {
namespace scn {

class Scene {
	std::list<Object> m_objects;
public:
	typedef util::ProxyIterator<decltype(m_objects), Scene> ObjectsIterator;
	Scene();
	~Scene();
	Scene(const Scene &other) = default;
	Scene(Scene &&other) = default;
	Scene& operator=(const Scene &other) = default;
	Scene& operator=(Scene &&other) = default;
	ObjectsIterator objectsBegin();
	ObjectsIterator objectsEnd();
	typedef util::ProxyContainer<&objectsBegin, &objectsEnd> Objects;
	Objects objects();
	ObjectsIterator objectsAdd(Object&& object);
	ObjectsIterator objectsAdd(const Object& object);
	void objectsRemove(ObjectsIterator it);
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_SCENE_H_ */
