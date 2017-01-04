#version 330 core

in vec2  Texcoord;

out vec4 color;

uniform sampler2D  quadTex;
uniform sampler2D  bloomBlur;

void main()
{
	const float gamma = 2.2;
 
    vec3 sceneColor = texture(quadTex, Texcoord).rgb;      
    vec3 bloomColor = texture(bloomBlur, Texcoord).rgb;
    
    sceneColor += bloomColor; // additive blending

    // also gamma correct while we're at it
    sceneColor = pow(sceneColor, vec3(1.0 / gamma));
	color = vec4(sceneColor, 1.0);
}
