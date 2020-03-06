/*
 * vertex_shaders.cpp
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#include "shaders.h"

namespace dse {
namespace subsys {
namespace gl31_impl {

const char fbVertexShader[] = R"glsl(
in vec3 pos;

void main() {
	gl_Position = vec4(pos, 1.f);
}
)glsl";

const char fbFragmentShader[] = R"glsl(
out vec4 fragColor;
uniform vec2 windowSize;

float ltos(float c) {
    if ( c <= .0031308f ) {
        c *= 12.92f;
    } else {
        float t = pow(c, 0.416667f);
        c = t + .055f * t - .055f;
    }
    return c;
}

void main() {
    vec2 uv = gl_FragCoord.xy / windowSize;
    /*float col = .5;

    if (all(greaterThan(uv, vec2(.25))) && all(lessThan(uv, vec2(.75)))) {

        float u = (uv.x - .25) * 2.;
        col = mod(gl_FragCoord.y - .5, 2.) == .0 ? u : 1. - u;
    }
    fragColor = vec4(vec3(col), 1.);*/
    fragColor = vec4(uv, .0f, 1.f);
    fragColor = vec4(ltos(fragColor.x), ltos(fragColor.y), ltos(fragColor.z), fragColor.w);
}
)glsl";

const char drawVertexShader[] = R"glsl(
in vec3 vPos;
in vec3 vNorm;
in vec3 vTang;
in vec2 vUV;

uniform vec3 iPos;
uniform vec4 qRot;
uniform vec3 scale;

out vec3 fNorm;
out vec3 fTang;
out vec3 fBitang;
out vec2 fUV;

vec4 qmul(vec4 a, vec4 b) {
    return vec4(
        a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
        a.w * b.w - dot(a.xyz, b.xyz)
    );
}

void main() {
	mat3 rot = mat3(
        cos(radians(30.f)), 0, sin(radians(30.f)),
        0, 1, 0,
        -sin(radians(30.f)), 0, cos(radians(30.f))
    );
    fNorm = vNorm;
    fTang = vTang;
    fUV = vUV;
    vec3 pos = vPos * scale;
    pos = qmul(qmul(qRot, vec4(pos, 1.f)), vec4(-qRot.xyz, qRot.w)).xyz;
    pos += iPos;
	gl_Position = vec4(pos.xyz, max(-pos.z + 2, 1));
}
)glsl";

const char drawFragmentShader[] = R"glsl(
in vec3 fNorm;
in vec3 fTang;
in vec3 fBitang;
in vec2 fUV;

out vec4 fragColor;

float ltos(float c) {
    if ( c <= .0031308f ) {
        c *= 12.92f;
    } else {
        float t = pow(c, 0.416667f);
        c = t + .055f * t - .055f;
    }
    return c;
}

void main() {
	fragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.f);
    fragColor = vec4(ltos(fragColor.x), ltos(fragColor.y), ltos(fragColor.z), fragColor.w);
}
)glsl";

}
}
}
