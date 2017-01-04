#version 330 core

layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 Texcoord0;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

const float NEAR = 0.1;
const float FAR = 100.0f;
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (-2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
}

void main()
{
	// Store the fragment position vector
	gPositionDepth.xyz = FragPos;
	// and store the z value
	gPositionDepth.w = FragPos.z; //LinearizeDepth(gl_FragCoord.z);

	// Store the per fragment normal
	gNormal = normalize(Normal);

	// the diffuse per-frgment color
	gAlbedoSpec.rgb = texture(diffuseTex, Texcoord0).rgb;
	// Store specular intensity in glAlbedo's alpha component
	gAlbedoSpec.a = texture(specularTex, Texcoord0).r;
}
