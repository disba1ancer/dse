in vec3 vPos;
in vec3 vNorm;
in vec3 vTang;
in vec2 vUV;

uniform vec3 camPos;
uniform vec4 camQRot;
uniform float invAspRatio;
uniform vec4 perspArgs;
uniform vec3 iPos;
uniform vec4 qRot;
uniform vec3 scale;

out vec3 fNorm;
out vec3 fTang;
out vec3 fBitang;
out vec2 fUV;
out vec3 fragPos;

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
    fragPos = pos;
    gl_Position = vec4(pos.x * invAspRatio, pos.y, pos.z * perspArgs.x + perspArgs.y, pos.z * perspArgs.z + perspArgs.w);
}
