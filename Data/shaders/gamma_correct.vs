#version 330 core

layout(location = 0) in vec2  position;
layout(location = 1) in float color0;

uniform mat4  mvp;

out vec3  fcolor;


void main()
{
	gl_Position = mvp * vec4(position, 0.f, 1.f);
	fcolor = vec3(color0, color0, color0);
}
