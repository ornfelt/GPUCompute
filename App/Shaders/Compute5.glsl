#version 460 core

// Uniform declarations required for the shader:
uniform sampler2D iChannel0; // Used in isColoured() for texturing
uniform sampler2D iChannel1; // Used in getColour() for texturing
uniform sampler2D iChannel2; // Used in mainImage() for an initial texture fetch
uniform sampler2D iChannel3; // Used in getColour() for a second texture lookup
uniform vec3 iResolution;    // Viewport resolution (e.g. width, height, 1.0)
uniform float iTime;         // Time in seconds

#define e 2.71

struct Orb {
    float value;
    vec2 center;
    float radius;
    vec3 colour;
};

struct Light {
    vec3 pos;
};

// Returns the intensity for the uv.
// Is only non-zero if the uv is in the "filled" part of the ball.
// We fake a fluid really poorly by basing the intensity off a noise function
// when we're really close to the top of the filled bit.
float isColoured(in vec2 uv, in Orb orb) {
    float orbBottom = orb.center.y - orb.radius;
    float level = orbBottom + orb.value * orb.radius * 2.0;
    if (distance(uv, orb.center) < orb.radius && (uv.y < level)) {
        if (uv.y / level > 0.99) {
            float textVar = texture(iChannel0, uv).x;
            if (textVar < 0.3) {
                return 0.0;
            }
            return textVar;
        }
        return 1.0;
    }
    return 0.0;
}
    
// Figures out the colour for the thing
vec3 getColour(in vec2 uv, in Orb orb) {
    vec3 colour = vec3(0.0);
    float borderDist = orb.radius - distance(uv, orb.center);
    float intensity = isColoured(uv, orb);
    if (intensity != 0.0) {
        // Fill that ball up
        colour = orb.colour * intensity;
        // Apply a texture to the ball (with a rudimentary UV mapping)
        vec2 distFromCent = uv - orb.center;
        float uvHeight = exp(sqrt(orb.radius - (pow(distFromCent.x, 2.0) + pow(distFromCent.y, 2.0))) / orb.radius) / exp(1.0);
        vec3 d = normalize(vec3(orb.center, 0.0) - vec3(uv, uvHeight));
        float u = (0.5 + atan(d.z, d.x) / (2.0 * 3.14159)) / (orb.radius * 0.5);
        float v = (0.5 - asin(d.y) / 3.14159) / (orb.radius * 0.5);
        colour *= texture(iChannel1, vec2(u + fract(iTime * 0.05),
                                          v + fract(iTime * 0.03))).x * 2.15;
        colour *= texture(iChannel3, vec2(u + fract(iTime * 0.001),
                                          v + fract(iTime * 0.04))).x * 2.2;
        // Apply a shadow based on the border distance
        colour /= pow(e, orb.radius * 0.2 / borderDist);
    }
    return colour;
}
    
// Add in a bevelled border
vec3 applyBorder(in vec2 uv, in Orb orb, vec3 currentColour) {
    vec3 colour = currentColour;
    float borderDist = orb.radius - distance(uv, orb.center);
    // Add in a bevelled border
    if (abs(borderDist) < 0.005) {
        colour = vec3(0.3, 0.3, 0.3);
        colour += vec3(0.7 - abs(borderDist) / 0.005) * 0.25;
    }
    return colour;
}
    
// Apply a specular highlight
vec3 applyHighlight(in vec2 uv, in Orb orb, in Light light, in vec3 eye) {
    vec3 colour = vec3(0.0);
    if (distance(orb.center, uv) >= orb.radius) {
        return colour;
    }
    
    vec2 distFromCent = uv - orb.center;    
    float uvHeight = sqrt(orb.radius - (pow(distFromCent.x, 2.0) + pow(distFromCent.y, 2.0)));
    vec3 uvw = vec3(uv, uvHeight);
    vec3 normal = normalize(vec3(uv, uvHeight) - vec3(orb.center, 0.0));
    vec3 orbToLight = normalize(light.pos - vec3(orb.center, 0.0));
    
    return vec3(pow(dot(reflect(normalize(uvw - light.pos), normal),
                        normalize(eye - uvw)), 55.0));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    // Convert fragCoord to uv coordinates.
    vec2 uv = fragCoord.xy / iResolution.x;
    vec3 view = vec3(0.0, 0.0, 10.0);
    // Start with a texture fetch (for background or effect)
    fragColor = texture(iChannel2, uv);
    
    Light l1 = Light(vec3(0.5, 0.7, 1.0));
    // Set up the first orb (e.g., health)
    vec2 center = vec2(0.25, 0.3);
    Orb health = Orb(abs(sin(iTime * 0.45)),
                     center,
                     0.2,
                     vec3(0.57, 0.0, 0.0));
    
    // Set up the second orb (e.g., mana)
    center = vec2(0.75, 0.3);
    Orb mana = Orb(abs(cos(iTime * 0.15)) + 0.01,
                   center,
                   0.2,
                   vec3(0.0, 0.3, 0.57));
    
    // Colour the first orb
    vec3 highLightValue = applyHighlight(uv, health, l1, view);
    vec3 redOrb = getColour(uv, health);
    if (redOrb.x > 0.0) {
        fragColor = vec4(redOrb, 1.0);
    }
    fragColor = vec4(applyBorder(uv, health, fragColor.xyz), 1.0);
    if (highLightValue.x > 0.0) {
        fragColor += vec4(highLightValue, 1.0);
    }
    
    // Colour the second orb
    highLightValue = applyHighlight(uv, mana, l1, view);
    vec3 blueOrb = getColour(uv, mana);
    if (blueOrb.z > 0.0) {
        fragColor = vec4(blueOrb, 1.0);
    }   
    fragColor = vec4(applyBorder(uv, mana, fragColor.xyz), 1.0);
    if (highLightValue.x > 0.0) {
        fragColor += vec4(highLightValue, 1.0);
    }
}

// Global output variable and entry point
out vec4 finalColor;
void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}
