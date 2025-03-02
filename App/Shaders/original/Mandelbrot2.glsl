// https://www.shadertoy.com/view/MltXz2
// Created by inigo quilez - iq/2016
//   https://www.youtube.com/c/InigoQuilez
//   https://iquilezles.org/
// I share this piece (art and code) here in Shadertoy and through its Public API, only for educational purposes. 
// You cannot use, sell, share or host this piece or modifications of it as part of your own commercial or non-commercial product, website or project.
// You can share a link to it or an unmodified screenshot of it provided you attribute "by Inigo Quilez, @iquilezles and iquilezles.org". 

// Visualizing the analytical solution for the bounds of the plane for
// which the dynamics of the iteration under the complex monic polynomial
// converges.
//
// The derivation of the formulas are here: 
//
// https://iquilezles.org/articles/mset1bulb
//
// Border:    c(w) = k^1/(1-k)·e^iw - k^k/(1-k)·e^ikw
// Inner rad: r = k^1/(1-k) - k^k/(1-k)
// Outer rad: R = 2^1/(k-1)
//
// Also, see here for the smooth iteration count formula: https://iquilezles.org/articles/msetsmooth

#define AA 2

//----------------------------------------------------------------
// complex numbers

vec2 cadd( vec2 a, float s ) { return vec2( a.x+s, a.y ); }
vec2 cmul( vec2 a, vec2 b )  { return vec2( a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x ); }
vec2 cdiv( vec2 a, vec2 b )  { float d = dot(b,b); return vec2( dot(a,b), a.y*b.x - a.x*b.y ) / d; }
vec2 cpow( vec2 z, float n ) { float r = length( z ); float a = atan( z.y, z.x ); return pow( r, n )*vec2( cos(a*n), sin(a*n) ); }
vec2 csqrt( vec2 z )         { float m = length(z); return sqrt( 0.5*vec2(m+z.x, m-z.x) ) * vec2( 1.0, sign(z.y) ); }
vec2 cconj( vec2 z )         { return vec2(z.x,-z.y); }

//----------------------------------------------------------------
// distance evaluators

float lengthSquared( in vec2 v ) { return dot(v,v); }
float sdSegmentSquared( vec2 p, vec2 a, vec2 b )
{
	vec2 pa = p-a, ba = b-a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return lengthSquared( pa - ba*h );
}

//----------------------------------------------------------------
// generalized mandelbrot set, with smooth iteration count

vec3 drawFractal( in float k, in vec2 fragCoord )
{
    vec3 col = vec3(0.0);
    
#if AA>1
    for( int m=0; m<AA; m++ )
    for( int n=0; n<AA; n++ )
    {
        vec2 o = vec2(float(m),float(n)) / float(AA) - 0.5;
        vec2 p = (-iResolution.xy + 2.0*(fragCoord+o))/iResolution.y;
#else    
        vec2 p = (-iResolution.xy + 2.0*fragCoord)/iResolution.y;
#endif

        vec2 c = p * 1.25;

        #if 0
        if( k==2.0 )
        {
        float c2 = dot(c, c);
        // skip computation inside M1 - https://iquilezles.org/articles/mset1bulb
    	if( 256.0*c2*c2 - 96.0*c2 + 32.0*c.x - 3.0 < 0.0 ) continue;
    	// skip computation inside M2 - https://iquilezles.org/articles/mset2bulb
    	if( 16.0*(c2+2.0*c.x+1.0) - 1.0 < 0.0 ) continue;
        }
        #endif
        
        const float threshold = 64.0;
        vec2 z = vec2( 0.0 );
        float it = 0.0;
        for( int i=0; i<100; i++ )
        {
            z = cpow(z, k) + c;
            if( dot(z,z)>threshold ) break;
            it++;
        }

        vec3 tmp = vec3(0.0);
        if( it<99.5 )
        {
            float sit = it - log2(log2(dot(z,z))/(log2(threshold)))/log2(k); // https://iquilezles.org/articles/msetsmooth
            tmp = 0.5 + 0.5*cos( 3.0 + sit*0.075*k + vec3(0.0,0.6,1.0));
        }
        
        col += tmp;
#if AA>1
    }
    col /= float(AA*AA);
#endif

	return col;
}

//----------------------------------------------------------------
// render

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float eps = 1.25/iResolution.y;
    vec2 p = (-iResolution.xy + 2.0*fragCoord)/iResolution.y;

    float time = iTime*0.25;
    

    vec2 c = p * 1.25;
    //float k = 2.0 + mod(floor(time), 10.0 );
    float k = 2.0 + floor(time) + smoothstep(0.8,1.0,fract(time));

    float m = pow( k, (1.0/(1.0-k)) );
    float n = pow( k, (k/(1.0-k)) );

    // draw fractal
    vec3 col = drawFractal( k, fragCoord );

    vec3 dcol = col;

    // draw convergence area
    {
        float d = 10.0;
        vec2 ow = vec2( m-n, 0.0 );
		for( int i=1; i<101; i++ )
        {
            float a = 6.283185*float(i)/200.0;
            vec2 w = m*vec2( cos(a), sin(a) ) - n*vec2( cos(k*a), sin(k*a) );

            d = min( d, sdSegmentSquared( vec2(c.x,abs(c.y)), ow, w ) );
            ow = w;
        }
        d = sqrt(d);
        dcol = mix( dcol, vec3(1.5,0.7,0.1), 1.0-smoothstep( 1.0*eps,6.0*eps,d) );
    }
    
    // draw inner radius
    {
        float r = m - n;
        float d = abs( length(c)-r );
        dcol = mix( dcol, vec3(1.0,1.0,1.0), 1.0-smoothstep( 1.0*eps,4.0*eps,d) );
    }
    
    // draw outer radius
    {
        float r = pow( 2.0, 1.0/(k-1.0) );
        float d = abs( length(c)-r );
        dcol = mix( dcol, vec3(1.0,1.0,1.0), 1.0-smoothstep( 0.0*eps,4.0*eps,d) );
    }
    
    float f = smoothstep( 0.2, 0.8, 1.0 - abs(2.0*fract(time)-1.0) );
    col = mix( col, dcol, f );
    

    fragColor = vec4( col, 1.0 );
}
