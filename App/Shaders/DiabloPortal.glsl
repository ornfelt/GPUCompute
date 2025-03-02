#version 460 core

// Uniform declarations (set these from your application)
uniform sampler2D iChannel0; // e.g. a noise texture
uniform vec3 iResolution;    // viewport resolution (width, height, depth)
uniform float iTime;         // elapsed time

// Preprocessor macros
#define time iTime*0.15
#define tau 6.2831853

// Helper functions
mat2 makem2(in float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat2(c, -s, s, c);
}

float noise(in vec2 x) {
    return texture(iChannel0, x * 0.01).x;
}

float fbm(in vec2 p) {	
    vec4 tt = fract(vec4(time * 2.0) + vec4(0.0, 0.25, 0.5, 0.75));
    vec2 p1 = p - normalize(p) * tt.x;
    vec2 p2 = vec2(1.0) + p - normalize(p) * tt.y;
    vec2 p3 = vec2(2.0) + p - normalize(p) * tt.z;
    vec2 p4 = vec2(3.0) + p - normalize(p) * tt.w;
    vec4 tr = vec4(1.0) - abs(tt - vec4(0.5)) * 2.0;
    float z = 2.0;
    vec4 rz = vec4(0.0);
    for (float i = 1.0; i < 4.0; i++) {
        rz += abs((vec4(noise(p1), noise(p2), noise(p3), noise(p4)) - vec4(0.5)) * 2.0) / z;
        z = z * 2.0;
        p1 *= 2.0;
        p2 *= 2.0;
        p3 *= 2.0;
        p4 *= 2.0;
    }
    return dot(rz, tr) * 0.25;
}

float dualfbm(in vec2 p) {
    // Get two fbm calls and displace the domain
    vec2 p2 = p * 0.7;
    vec2 basis = vec2(fbm(p2 - time * 1.6), fbm(p2 + time * 1.7));
    basis = (basis - 0.5) * 0.2;
    p += basis;
    // Return fbm for coloring
    return fbm(p);
}

float circ(vec2 p) {
    float r = length(p);
    r = log(sqrt(r));
    return abs(mod(r * 2.0, tau) - 4.54) * 3.0 + 0.5;
}

// Shadertoy-style mainImage function
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    // Normalize and adjust coordinates
    vec2 p = fragCoord.xy / iResolution.xy - 0.5;
    p.x *= iResolution.x / iResolution.y;
    p *= 4.0;
    
    float rz = dualfbm(p);
    
    // Apply ring-like distortion several times
    rz *= abs(-circ(vec2(p.x / 4.2, p.y / 7.0)));
    rz *= abs(-circ(vec2(p.x / 4.2, p.y / 7.0)));
    rz *= abs(-circ(vec2(p.x / 4.2, p.y / 7.0)));
    
    // Final color calculation
    vec3 col = vec3(0.1, 0.1, 0.4) / rz;
    col = pow(abs(col), vec3(0.99));
    fragColor = vec4(col, 1.0);
}

// Declare a global output variable and call mainImage
out vec4 finalColor;
void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}
