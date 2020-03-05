/*
 * ObjectInstance.h
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_
#define SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_

#include "../../scn/Object.h"

namespace dse {
namespace subsys {
namespace gl31_impl {

class ObjectInstance {
	scn::Object* object;
	unsigned lastVersion;
public:
	ObjectInstance();
	~ObjectInstance() = default;
	ObjectInstance(const ObjectInstance &other) = default;
	ObjectInstance(ObjectInstance &&other) = default;
	ObjectInstance& operator=(const ObjectInstance &other) = default;
	ObjectInstance& operator=(ObjectInstance &&other) = default;
};

} /* namespace gl31_impl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL31_IMPL_OBJECTINSTANCE_H_ */
