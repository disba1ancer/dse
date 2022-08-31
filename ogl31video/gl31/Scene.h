/*
 * Scene.h
 *
 *  Created on: 20 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_SCENE_H_
#define SUBSYS_GL31_IMPL_SCENE_H_

#include <dse/core/Scene.h>

namespace dse::ogl31rbe::gl31 {

class Scene {
	core::Scene* scene;
public:
	Scene(core::Scene* scene);
	~Scene() = default;
	Scene(const Scene &other) = default;
	Scene(Scene &&other) = default;
	Scene& operator=(const Scene &other) = default;
	Scene& operator=(Scene &&other) = default;
};

} /* namespace dse::ogl31rbe::gl31 */

#endif /* SUBSYS_GL31_IMPL_SCENE_H_ */
