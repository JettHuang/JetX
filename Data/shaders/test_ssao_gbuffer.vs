#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord0;


out vec3 FragPos;
out vec3 Normal;
out vec2 Texcoord0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	vec4 viewPos = view * model * vec4(position, 1.0);
	gl_Position = projection * viewPos;

	FragPos = viewPos.xyz;
	Normal = transpose(inverse(mat3(view * model))) * normal;
	Texcoord0 = texcoord0;
}

