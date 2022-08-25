in vec3 fNorm;
in vec3 fTang;
in vec3 fBitang;
in vec2 fUV;
in vec3 vWorldPos;

uniform vec2 windowSize;
layout(std140) uniform Camera {
    mat4 viewProj;
    vec4 pos;
} camera;
layout(std140) uniform Material {
    vec4 color;
} material;
uniform sampler2D diffuse;

out vec4 fragColor;

vec4 defaultTexture(vec2 uv) {
    float t = step(0.f, (uv.x - .5f) * (uv.y - .5f));
    return vec4(clamp(vec3(t, .0f, t), .09325f, .90675f), 1.f);
}

void main() {
    vec4 lightCol = vec4(1.f, 1.f, .75f, 1.f);
    vec3 lightDir = normalize(vec3(1.f, 1.f, -1.f));
    vec4 diffuseCol = mix(texture(diffuse, fUV), vec4(material.color.rgb, 1.f), material.color.a);
    vec3 viewDir = normalize(camera.pos.xyz - vWorldPos);
    float brightness = dot(fNorm, -lightDir);
    vec3 specDir = 2 * fNorm * brightness + lightDir;
    fragColor = diffuseCol * vec4(.03125f, .03125f, .0625f, 1.f) + max(0.f, brightness) * lightCol * diffuseCol + pow(max(0.f, dot(specDir, viewDir)), 8.f) * lightCol * diffuseCol;
}
