#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 Texcoord0;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

void main()
{
	// Store the fragment position vector
	gPosition = FragPos;
	// Store the per fragment normal
	gNormal = normalize(Normal);
	// the diffuse per-frgment color
	gAlbedoSpec.rgb = texture(diffuseTex, Texcoord0).rgb;
	// Store specular intensity in glAlbedo's alpha component
	gAlbedoSpec.a = texture(specularTex, Texcoord0).r;
}
