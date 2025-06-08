/*
 * binds.h
 *
 *  Created on: 3 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_BINDS_H_
#define SUBSYS_GL31_IMPL_BINDS_H_

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <dse/math/vec.h>
#include <dse/math/mat.h>

namespace dse::ogl31rbe::gl31 {

enum InputParams {
    Position, Normal, Tangent, UV, BTangSign
};

enum OutputParams {
    FragmentColor
};

inline constexpr auto attachment(OutputParams unit) {
    return ::gl::GL_COLOR_ATTACHMENT0 + unit;
}

enum UniformIndices {
    ObjectInstanceBind,
    CameraBind,
    MaterialBind
};

struct ObjectMaterialUniform {
    alignas(alignof(float) * 4) math::vec4 color;
};

struct ObjectInstanceUniform {
    alignas(alignof(float) * 4) math::mat3x4 transform;
};

struct CameraUniform {
    alignas(alignof(float) * 4) math::mat4 viewProj;
    alignas(alignof(float) * 4) math::vec4 pos;
};

enum TextureUnits {
    NullImageUnit,
    PostProcColor,
    PostProcDepth,
    DrawDiffuse = 1,
    DrawNormalMap,
};

inline constexpr auto texture(TextureUnits unit) {
    return ::gl::GL_TEXTURE0 + unit;
}

} /* namespace dse::ogl31rbe::gl31 */

#endif /* SUBSYS_GL31_IMPL_BINDS_H_ */
