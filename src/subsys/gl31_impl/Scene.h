/*
 * Scene.h
 *
 *  Created on: 20 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_SCENE_H_
#define SUBSYS_GL31_IMPL_SCENE_H_

#include "scn/Scene.h"

namespace dse {
namespace subsys {
namespace gl31_impl {

class Scene {
	scn::Scene* scene;
public:
	Scene(scn::Scene* scene);
	~Scene() = default;
	Scene(const Scene &other) = default;
	Scene(Scene &&other) = default;
	Scene& operator=(const Scene &other) = default;
	Scene& operator=(Scene &&other) = default;
};

} /* namespace gl31_impl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL31_IMPL_SCENE_H_ */
