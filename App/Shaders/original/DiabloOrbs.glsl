// https://www.shadertoy.com/view/llcSRf

#define e 2.71
struct Orb{
    float value;
    vec2 center;
    float radius;
    vec3 colour;
};
    
struct Light{
	vec3 pos;
};
    
// Returns the intensity for the uv.
// Is only non-zero if the uv is in the "filled" part of the ball.
// We fake a fluid really poorly by basing the intensity off a noise function
// when we're really close to the top of the filled bit.
float isColoured(in vec2 uv, in Orb orb){
    float orbBottom = orb.center.y - orb.radius;
    float level = orbBottom + orb.value * orb.radius * 2.;
    
    if (distance(uv,orb.center) < orb.radius && (uv.y < level)) {
        if (uv.y / level > .99){
        	float textVar = texture(iChannel0, uv).x;
            if (textVar < .3){
                return 0.;
            }
            return textVar;
        }
		return 1.;
    }
	return 0.;
}
    
// Figures out the colour for the thing
vec3 getColour(in vec2 uv, in Orb orb){
    vec3 colour = vec3(0.);
    float borderDist = orb.radius - distance(uv, orb.center);
    float intensity = isColoured(uv, orb);
	if (intensity != 0.){
        // Fill that ball up homie
    	colour = vec3(orb.colour * intensity);
        // Apply a texture to the ball
        // Do some proper uv mapping: 
        // https://en.wikipedia.org/wiki/UV_mapping#Finding_UV_on_a_sphere
        vec2 distFromCent = uv - orb.center;
        float uvHeight = exp(sqrt(orb.radius - (pow(distFromCent.x,2.) + pow(distFromCent.y,2.))) 
            / orb.radius) / exp(1.);
        vec3 d = normalize(vec3(orb.center, 0.) - vec3(uv, uvHeight));        
        float u = (.5 + atan(d.z, d.x)
            			/ (2. * 3.14159)) / orb.radius / .5;
        float v = (.5 - asin(d.y) / 3.14159) / orb.radius / .5;
        colour *= vec3(texture(iChannel1, vec2(u + fract(iTime *.05),
                                                 v + fract(iTime * .03))).x)
            		* 2.15;
        colour *= vec3(texture(iChannel3, vec2(u + fract(iTime * .001),
                                                 v + fract(iTime * .04))).x)
            		* 2.2;
        // Add some sort of shadow to the coloured bit
        // TODO: apply some highlight repsonsiveness
        colour /= pow(2.71, orb.radius * .2 / borderDist);
    }
    
    return colour;
}

// Add in a bevelled border
vec3 applyBorder(in vec2 uv, in Orb orb, vec3 currentColour){
    vec3 colour = currentColour;
    float borderDist = orb.radius - distance(uv, orb.center);
	// Add in a bevelled border
    if (abs(borderDist) < .005) {
        colour = vec3(0.3,0.3,0.3);
        colour += vec3(.7 - abs(borderDist)/.005) * .25;
    }
    return colour;
}

// Apply a specular highlight
// https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Specular_Highlights
vec3 applyHighlight(in vec2 uv, in Orb orb, in Light light, in vec3 eye){
    vec3 colour = vec3(0.);
    if (distance(orb.center, uv) >= orb.radius){
        return colour;
     }
    
    vec2 distFromCent = uv - orb.center;    
    float uvHeight = sqrt(orb.radius - (pow(distFromCent.x,2.) + pow(distFromCent.y,2.)));
    vec3 uvw = vec3(uv, uvHeight);
    vec3 normal = normalize(vec3(uv, uvHeight) - vec3(orb.center, 0.));
    vec3 orbToLight = normalize(light.pos - vec3(orb.center, 0.));
    
    return vec3(pow(dot(reflect(normalize(uvw - light.pos),
                                normal),
                        normalize(eye - uvw)),
                    55.));
}



void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.x;
    vec3 view = vec3(0., 0., 10.);
    fragColor = texture(iChannel2, uv);
    //fragColor = vec4(0.);
    
    Light l1 = Light(vec3(0.5,.7, 1.));
    // Set up the two orbs
    vec2 center = vec2(.25, .3);
    Orb health = Orb(abs(sin(iTime * .45)),
                     center,
                     .2,
                     vec3(.57, 0., 0.));
    
    center = vec2(.75, .3);
    Orb mana = Orb(abs(cos(iTime * .15))+ .01,
                   center,
                   .2,
                   vec3(0., 0.3, .57));
    
    // Colour the first orb
    vec3 highLightValue = applyHighlight(uv, health, l1, view);
    vec3 redOrb = vec3(getColour(uv, health));
    if (redOrb.x > 0.0){
        fragColor = vec4(redOrb, 1.);
        
    }
    fragColor = vec4(applyBorder(uv, health, fragColor.xyz), 1.);        
    if (highLightValue.x > 0.){
    	fragColor += vec4(highLightValue, 1.);
    }
    
    // Colour the second orb
    highLightValue = applyHighlight(uv, mana, l1, view);
    vec3 blueOrb = vec3(getColour(uv, mana));
    if (blueOrb.z > 0.0){
        fragColor = vec4(blueOrb, 1.);        
    }   
    fragColor = vec4(applyBorder(uv, mana, fragColor.xyz), 1.);        
    if (highLightValue.x > 0.){
    	fragColor += vec4(highLightValue, 1.);
    }
}
