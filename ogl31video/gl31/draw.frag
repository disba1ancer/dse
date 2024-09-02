in vec3 fNorm;
in vec3 fTang;
in vec3 fBitang;
in vec2 fUV;
in vec3 viewDir;
in vec3 lightDir;

uniform vec2 windowSize;
layout(std140) uniform Camera {
    mat4 viewProj;
    vec4 pos;
} camera;
layout(std140) uniform Material {
    vec4 color;
} material;
uniform sampler2D diffuse;
uniform sampler2D normalMap;

out vec4 fragColor;

const float PI = 3.141593f;

vec4 defaultTexture(vec2 uv) {
    float t = step(0.f, (uv.x - .5f) * (uv.y - .5f));
    return vec4(clamp(vec3(t, .0f, t), .09325f, .90675f), 1.f);
}

const vec3 lightCol = vec3(1.f, 1.f, 1.f);
const vec3 ambientColor = vec3(.03125f, .03125f, .0625f);
// const vec3 ambientColor = vec4(.0f, .0f, .0f);
const float sunAng = 0.004363f;
const float sunCos = 0.99999f;

float getBlurCoef(float rA, float rB, float x)
{
    return max(0, min(rA, x + rB) - max(-rA, x - rB)) * .5 / rB;
}

vec3 getAmbientColor(vec3 normal, vec3 half, float blur)
{
    float cos2NH = dot (normal, half);
    if (cos2NH <= 0.f) {
        return ambientColor;
    }
    blur *= blur;
    cos2NH *= cos2NH;
    return mix(ambientColor, lightCol, (exp((cos2NH - 1.f) / (cos2NH * blur)) / (PI * blur * cos2NH * cos2NH)));
}

float calcRefr(float k)
{
    k = (1 - k) / (1 + k);
    return k * k;
}

float IORtoBaseR(float ior)
{
    float r0 = (1 - ior) / (1 + ior);
    return r0 * r0;
}

vec3 fresnel(float cosVN, vec3 r0)
{
    float icosVN = 1.f - cosVN;
    float icos4VN = icosVN * icosVN;
    icos4VN *= icos4VN;
    return mix(vec3(icos4VN * icosVN), vec3(1.f), r0);
}

void main() {
    vec4 diffuseCol = texture(diffuse, fUV);
    diffuseCol = mix(material.color, diffuseCol, diffuseCol.a);
    vec3 normal = normalize(texture(normalMap, fUV).xyz * 2.f - 1.f);
    float normK = 1.f / sqrt(dot(fNorm, fNorm));
    vec3 nLightDir = normalize(lightDir);
    vec3 nViewDir = normalize(viewDir);
    mat3 tbn = mat3(fTang * normK, fBitang * normK, fNorm * normK);
    vec3 nNorm = tbn * normal;
    float brightness = max(0.f, dot(nNorm, nLightDir));
    vec3 specDir = 2 * nNorm * dot(nNorm, nViewDir) - nViewDir;
    vec3 reflColor = mix(ambientColor, getAmbientColor(nNorm, normalize(.5f * (nViewDir + nLightDir)), .5f), brightness);
    float cosVN = max(0.f, dot(nViewDir, nNorm));
    vec3 kF = fresnel(cosVN, vec3(diffuseCol.xyz));
    fragColor = vec4(reflColor * kF, 1.f); //mix(mix(ambientColor, lightCol, brightness) * diffuseCol, reflColor, kF);
    //fragColor = diffuseCol * ambientColor + max(0.f, brightness) * lightCol * diffuseCol + pow(max(0.f, dot(specDir, nViewDir)), 16.f) * lightCol * diffuseCol;
    //fragColor = vec4(tbn[2] * .5f + .5f, 1.f);reflColor
}
