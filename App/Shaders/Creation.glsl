#version 460 core

// Uniform declarations required for the shader:
uniform vec3 iResolution;   // viewport resolution (in pixels)
uniform float iTime;        // shader playback time (in seconds)
uniform vec4 iMouse;        // mouse coordinates: (current x, current y, click x, click y)
uniform sampler2D iChannel0; // first texture channel (e.g. land map)
uniform sampler2D iChannel1; // second texture channel (e.g. pressure data)
uniform sampler2D iChannel2; // third texture channel (e.g. flow data)
uniform vec2 PASS3;         // offset vector for iChannel1 sampling
uniform vec2 PASS4;         // offset vector for iChannel1 in the alternate branch
uniform float MAPRES;       // a scale factor for the mapping resolution

// http://www.pouet.net/prod.php?which=57245
// If you intend to reuse this shader, please add credits to 'Danilo Guanabara'

#define t iTime
#define r iResolution.xy

// Declare the global output variable.
out vec4 fragColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    vec3 c;
    float l, z = t;
    for(int i = 0; i < 3; i++) {
        vec2 uv, p = fragCoord.xy / r;
        uv = p;
        p -= 0.5;
        p.x *= r.x / r.y;
        z += 0.07;
        l = length(p);
        uv += p / l * (sin(z) + 1.0) * abs(sin(l * 9.0 - z - z));
        c[i] = 0.01 / length(mod(uv, 1.0) - 0.5);
    }
    fragColor = vec4(c / l, t);
}

// The actual entry point for the fragment shader.
void main() {
    mainImage(fragColor, gl_FragCoord.xy);
}
