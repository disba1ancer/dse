in vec3 vPos;
in vec3 vNorm;
in vec3 vTang;
in vec2 vUV;
in float vBTangSign;

layout(std140) uniform Camera {
    mat4 viewProj;
    vec4 pos;
} camera;
layout(std140) uniform ObjectInstance {
    layout(row_major) mat4x3 transform;
} object;

out vec3 fNorm;
out vec3 fTang;
out vec3 fBitang;
out vec2 fUV;
out vec3 viewDir;
out vec3 lightDir;

vec4 qmul(vec4 a, vec4 b) {
    return (a.wwwx * b.xyzx + a.xyzy * b.wwwy + a.yzxz * b.zxyz - a.zxyw * b.yzxw) * vec4(1,1,1,-1);
//    return vec4(
//        a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
//        a.w * b.w - dot(a.xyz, b.xyz)
//    );
}

vec4 qinv(vec4 q) {
    return vec4(-q.xyz, q.w);
}

vec3 vecrotquat(vec3 vec, vec4 quat) {
    return qmul(qmul(quat, vec4(vec, 0.f)), qinv(quat)).xyz;
}

void main() {
    fNorm = (object.transform * vec4(vNorm, 0.f)).xyz;
    fTang = (object.transform * vec4(vTang, 0.f)).xyz;
    fBitang = cross(fNorm, fTang) * vBTangSign;
    fUV = vUV;
    vec3 pos = (object.transform * vec4(vPos, 1.f)).xyz;
    viewDir = camera.pos.xyz - pos;
    gl_Position = camera.viewProj * vec4(pos, 1.f);
    lightDir = vec3(1.f, 1.f, -1.f);
}
