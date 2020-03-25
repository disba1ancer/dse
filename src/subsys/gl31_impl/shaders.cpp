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

uniform vec3 camPos;
uniform vec4 camQRot;
uniform float invFocLen;
uniform float zNear;
uniform float zFar;
uniform float aspRatio;
uniform vec3 iPos;
uniform vec4 qRot;
uniform vec3 scale;
/*w = z * invFocLen;
z = -(2 * z + zSum) * zInvDiff * w;*/

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

vec4 qinv(vec4 q) {
    return vec4(-q.xyz, q.w);
}

vec3 vecrotquat(vec3 vec, vec4 quat) {
    return qmul(qmul(quat, vec4(vec, 0.f)), qinv(quat)).xyz;
}

void main() {
    vec4 q = qmul(qinv(camQRot), qRot);
    fNorm = vecrotquat(vNorm, q);
    fTang = vecrotquat(vTang, q);
    fUV = vUV;
    vec3 pos = vPos * scale;
    pos = vecrotquat(pos, qRot);
    pos += iPos - camPos;
    pos = vecrotquat(pos, qinv(camQRot));
    float zSum = zNear + zFar;
    float zInvDiff = 1 / (zFar - zNear);
    float w = -pos.z * invFocLen;
    gl_Position = vec4(pos.x / aspRatio, pos.y, zSum * invFocLen * (-pos.z - zNear) * zInvDiff - invFocLen * zNear, w);
}
)glsl";

const char drawFragmentShader[] = R"glsl(
in vec3 fNorm;
in vec3 fTang;
in vec3 fBitang;
in vec2 fUV;

uniform vec2 windowSize;
uniform float invFocLen;
uniform float zNear;
uniform float zFar;
uniform float aspRatio;

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

vec4 defaultTexture(vec2 uv) {
    float t = abs(dot(clamp((fract(uv) - .5f) * 256.f, -.5f, .5f), vec2(1.f)));
    return vec4(clamp(vec3(t, .0f, t), .09325f, .90675f), 1.f);
}

void main() {
    vec3 lightDir = -normalize(vec3(1.f, -1.f, 0.f));
    vec4 lightCol = vec4(1.f, 1.f, .75f, 1.f);
    vec4 diffuseCol = defaultTexture(fUV);
    vec3 viewDir = -normalize(vec3((gl_FragCoord.xy * 2.f / windowSize - 1.f) * vec2(invFocLen * aspRatio, invFocLen), -1));
    float brightness = dot(fNorm,  lightDir);
    vec3 specDir = 2 * fNorm * brightness - lightDir;
    //fragColor = defaultTexture(fUV) * lightCol * brightness + lightCol * dt;
    fragColor = diffuseCol * vec4(vec3(.03125f), 1.f) + max(0.f, brightness) * lightCol * diffuseCol + pow(max(0.f, dot(specDir, viewDir)), 8.f) * lightCol * diffuseCol;
    //fragColor = vec4(vec3(max(0.f, dot(specDir, viewDir))), 1.f);
    fragColor = vec4(ltos(fragColor.x), ltos(fragColor.y), ltos(fragColor.z), fragColor.w);
}
)glsl";

}
}
}
