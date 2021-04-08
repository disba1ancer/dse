/*
 * binds.h
 *
 *  Created on: 3 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_BINDS_H_
#define SUBSYS_GL31_IMPL_BINDS_H_

#include "math/vec.h"
#include "math/mat.h"

namespace dse::renders::gl31 {

enum InputParams {
	Position, Normal, Tangent, UV
};

enum OutputParams {
	FragmentColor
};

enum UniformIndices {
	ObjectInstanceBind,
	CameraBind
};

struct ObjectInstanceUniform {
//	alignas(alignof(float) * 4) math::vec3 iPos;
//	alignas(alignof(float) * 4) math::vec4 qRot;
//	alignas(alignof(float) * 4) math::vec3 scale;
	alignas(alignof(float) * 4) math::mat3x4 transform;
};

struct CameraUniform {
	alignas(alignof(float) * 4) math::mat4 viewProj;
	alignas(alignof(float) * 4) math::vec4 pos;
};

} /* namespace dse::renders::gl31 */

#endif /* SUBSYS_GL31_IMPL_BINDS_H_ */
