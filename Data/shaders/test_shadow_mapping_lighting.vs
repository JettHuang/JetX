#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

// declare an interface block
out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 shadowClipPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 ShadowVP;


void main()
{
	vec4 worldPos = model * vec4(position, 1.0f);
	vec4 worldNormal = model * vec4(normal, 0.f);

	gl_Position = projection * view * worldPos;
	vs_out.FragPos = worldPos.xyz;
	vs_out.Normal = worldNormal.xyz;
	vs_out.TexCoords = texCoords;
	vs_out.shadowClipPos = ShadowVP * worldPos;
}
