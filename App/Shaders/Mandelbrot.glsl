#version 460 core

// Declare required uniforms.
uniform vec3 iResolution; // For example: (windowWidth, windowHeight, 1.0)
uniform float iTime;      // Animation time in seconds

// Compute the distance to the Mandelbrot set.
float distanceToMandelbrot( in vec2 c )
{
    #if 1
    {
        float c2 = dot(c, c);
        // Skip computation inside M1 - see https://iquilezles.org/articles/mset1bulb
        if( 256.0 * c2 * c2 - 96.0 * c2 + 32.0 * c.x - 3.0 < 0.0 ) return 0.0;
        // Skip computation inside M2 - see https://iquilezles.org/articles/mset2bulb
        if( 16.0 * (c2 + 2.0 * c.x + 1.0) - 1.0 < 0.0 ) return 0.0;
    }
    #endif

    // Iterate
    float di = 1.0;
    vec2 z = vec2(0.0);
    float m2 = 0.0;
    vec2 dz = vec2(0.0);
    for( int i = 0; i < 300; i++ )
    {
        if( m2 > 1024.0 ) { di = 0.0; break; }
        // Z' -> 2·Z·Z' + 1
        dz = 2.0 * vec2(z.x * dz.x - z.y * dz.y, z.x * dz.y + z.y * dz.x) + vec2(1.0, 0.0);
        // Z -> Z² + c
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        m2 = dot(z, z);
    }
    // Distance: d(c) = |Z|·log|Z|/|Z'|
    float d = 0.5 * sqrt(dot(z, z) / dot(dz, dz)) * log(dot(z, z));
    if( di > 0.5 ) d = 0.0;
    return d;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Convert fragCoord to normalized coordinate system.
    vec2 p = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    
    // Animation factor.
    float tz = 0.5 - 0.5 * cos(0.225 * iTime);
    float zoo = pow(0.5, 13.0 * tz);
    // Offset the Mandelbrot coordinate.
    vec2 c = vec2(-0.05, 0.6805) + p * zoo;
    
    // Compute distance to the Mandelbrot set.
    float d = distanceToMandelbrot(c);
    // Soft color based on distance.
    d = clamp(pow(4.0 * d / zoo, 0.2), 0.0, 1.0);
    
    vec3 col = vec3(d);
    fragColor = vec4(col, 1.0);
}

// Fragment shader entry point.
out vec4 finalColor;
void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}
