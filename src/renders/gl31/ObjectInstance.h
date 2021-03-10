/*
 * ObjectInstance.h
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_
#define SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_

#include <memory>
#include "scn/Object.h"
#include "MeshInstance.h"

namespace dse::renders::gl31 {

struct ObjectInstance {
	scn::Object* object;
	std::unique_ptr<MeshInstance, MeshInstance::Deleter> mesh;
	unsigned lastVersion;
};

} /* namespace dse::renders::gl31 */

#endif /* SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_ */
