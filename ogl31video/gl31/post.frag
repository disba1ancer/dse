out vec4 outColor;
uniform vec2 windowSize;
uniform sampler2D colorBuffer;
uniform sampler2DShadow depthBuffer;

float ltos(float c) {
    if ( c <= .0031308f ) {
        c *= 12.92f;
    } else {
        float t = pow(c, 0.416667f);
        c = t + .055f * t - .055f;
    }
    return c;
}

vec3 ltos(vec3 c) {
    return vec3(ltos(c.r), ltos(c.g), ltos(c.b));
}

vec4 ltos(vec4 c) {
    return vec4(ltos(c.rgb), c.a);
}

void main() {
    outColor = ltos(clamp(texelFetch(colorBuffer, ivec2(gl_FragCoord.xy), 0), 0.f, 1.f));
}
