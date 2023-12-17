#ifndef DSE_MATH_COLOR_H
#define DSE_MATH_COLOR_H

#include <cmath>
#include "vec.h"

namespace dse::math {

inline auto ToLinearColor(float color) -> float
{
    if ( color <= .04045f ) {
        color /= 12.92f;
    } else {
        color = std::pow((color + .055f) / 1.055f, 2.4f);
    }
    return color;
}

inline auto ToLinearColor(const vec4& color) -> vec4
{
    vec4 result;
    for (int i = 0; i < 3; ++i) {
        result[i] = ToLinearColor(color[i]);
    }
    result[3] = color[3];
    return result;
}

inline float ToSRGBColor(float color) {
    if ( color <= .0031308f ) {
        color *= 12.92f;
    } else {
        color = 1.055f * pow(color, 1 / 2.4f) - .055f;
    }
    return color;
}

inline auto ToSRGBColor(const vec4& color) -> vec4
{
    vec4 result;
    for (int i = 0; i < 3; ++i) {
        result[i] = ToSRGBColor(color[i]);
    }
    result[3] = color[3];
    return result;
}

} // namespace dse::math

#endif // COLOR_H
