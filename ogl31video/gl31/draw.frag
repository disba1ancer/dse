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

vec4 defaultTexture(vec2 uv) {
    float t = step(0.f, (uv.x - .5f) * (uv.y - .5f));
    return vec4(clamp(vec3(t, .0f, t), .09325f, .90675f), 1.f);
}

const vec4 lightCol = vec4(1.f, 1.f, 1.f, 1.f);
const vec4 ambientColor = vec4(.03125f, .03125f, .0625f, 1.f);
// const vec4 ambientColor = vec4(.0f, .0f, .0f, 1.f);
const float sunAng = 0.004363f;
const float sunCos = 0.999999f;

float getBlurCoef(float rA, float rB, float x)
{
    return max(0, min(rA, x + rB) - max(-rA, x - rB)) * .5 / rB;
}

vec4 getAmbientColor(vec3 _lightDir, vec3 reflDir, float blur)
{
    float angle = acos(dot(reflDir, _lightDir));
    // return mix(lightCol, ambientColor, step(sunAng, angle));
    return lightCol * getBlurCoef(sunAng, sunAng / 16/*blur * 3.141593f * .5f*/, angle);
}

float calcRefr(float k)
{
    k = (1 - k) / (1 + k);
    return k * k;
}

void main() {
    float refrK = calcRefr(1.5f);
    vec4 diffuseCol = texture(diffuse, fUV);
    diffuseCol = mix(material.color, diffuseCol, diffuseCol.a);
    vec3 normal = normalize(texture(normalMap, fUV).xyz * 2.f - 1.f);
    float normK = 1.f / sqrt(dot(fNorm, fNorm));
    vec3 nLightDir = normalize(lightDir);
    vec3 nViewDir = normalize(viewDir);
    mat3 tbn = mat3(fTang * normK, fBitang * normK, fNorm * normK);
    vec3 nNorm = fNorm * normK;
    float brightness = max(0.f, dot(nNorm, nLightDir));
    float specCos = dot(nViewDir, nNorm);
    float specCos1 = 1.f - specCos;
    vec3 specDir = 2 * nNorm * dot(nNorm, nViewDir) - nViewDir;
    vec4 reflColor = getAmbientColor(nLightDir, specDir, 0.f);
    float reflK = refrK + (1 - refrK) * pow(specCos1, 5.f);
    fragColor = getAmbientColor(nLightDir, nNorm, .0078125f);//mix(mix(ambientColor, lightCol, brightness) * diffuseCol, reflColor, reflK);
    //fragColor = diffuseCol * ambientColor + max(0.f, brightness) * lightCol * diffuseCol + pow(max(0.f, dot(specDir, nViewDir)), 16.f) * lightCol * diffuseCol;
    //fragColor = vec4(tbn[2] * .5f + .5f, 1.f);
}
