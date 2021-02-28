in vec3 fNorm;
in vec3 fTang;
in vec3 fBitang;
in vec2 fUV;
in vec3 fragPos;
in vec3 lightDir;

uniform vec2 windowSize;
uniform vec4 matColor;

out vec4 fragColor;

vec4 defaultTexture(vec2 uv) {
    float t = step(0.f, (uv.x - .5f) * (uv.y - .5f));
    return vec4(clamp(vec3(t, .0f, t), .09325f, .90675f), 1.f);
}

void main() {
    vec4 lightCol = vec4(1.f, 1.f, .75f, 1.f);
    vec4 diffuseCol = mix(defaultTexture(fUV), vec4(matColor.rgb, 1.f), matColor.a);
    vec3 viewDir = -normalize(fragPos);
    float brightness = dot(fNorm, -lightDir);
    vec3 specDir = 2 * fNorm * brightness + lightDir;
    vec4 color = diffuseCol * vec4(.03125f, .03125f, .0625f, 1.f) + max(0.f, brightness) * lightCol * diffuseCol + pow(max(0.f, dot(specDir, viewDir)), 8.f) * lightCol * diffuseCol;
    fragColor = clamp(color, 0.f, 1.f);
}
