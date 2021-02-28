out vec4 fragColor;
uniform vec2 windowSize;
uniform sampler2DRect colorBuffer;
uniform sampler2DRectShadow depthBuffer;

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
//    vec2 uv = gl_FragCoord.xy / windowSize;
//    /*float col = .5;
//
//    if (all(greaterThan(uv, vec2(.25))) && all(lessThan(uv, vec2(.75)))) {
//
//        float u = (uv.x - .25) * 2.;
//        col = mod(gl_FragCoord.y - .5, 2.) == .0 ? u : 1. - u;
//    }
//    fragColor = vec4(vec3(col), 1.);*/
//    fragColor = vec4(uv, .0f, 1.f);
//    //fragColor = vec4(ltos(fragColor.rgb), fragColor.a);
    fragColor = ltos(texture(colorBuffer, gl_FragCoord.xy));
}
