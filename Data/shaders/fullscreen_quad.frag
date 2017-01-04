#version 330 core

in vec2  Texcoord;

out vec4 color;

uniform sampler2D  sceneTex;


void main()
{
	color = texture(sceneTex, Texcoord);
	float average = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;

	color = vec4(average, average, average, 1.f);
}

