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

void main() {
    vec4 lightCol = vec4(1.f, 1.f, 1.f, 1.f);
    vec4 diffuseCol = texture(diffuse, fUV);
    diffuseCol = mix(material.color, diffuseCol, diffuseCol.a);
    vec3 normal = normalize(texture(normalMap, fUV).xyz * 2.f - 1.f);
    float normK = 1.f / sqrt(dot(fNorm, fNorm));
    vec3 nLightDir = normalize(lightDir);
    vec3 nViewDir = normalize(viewDir);
    mat3 tbn = mat3(fTang * normK, fBitang * normK, fNorm * normK);
    vec3 nNorm = tbn * normal;
    float brightness = dot(nNorm, -nLightDir);
    vec3 specDir = 2 * nNorm * brightness + nLightDir;
    fragColor = diffuseCol * vec4(.03125f, .03125f, .0625f, 1.f) + max(0.f, brightness) * lightCol * diffuseCol + pow(max(0.f, dot(specDir, nViewDir)), 16.f) * lightCol * diffuseCol;
    //fragColor = vec4(tbn[2] * .5f + .5f, 1.f);
}
