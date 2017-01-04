#version 330 core

layout(location = 0)  in vec2  position;
layout(location = 1)  in vec2  texcoord;

uniform mat4  mvp;

out vec2 Texcoord;


void main()
{
	gl_Position = mvp * vec4(position, 0.f, 1.f);
	Texcoord = texcoord;
}

