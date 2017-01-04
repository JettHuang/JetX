#version 330 core

in vec2 Texcoord0;

out vec4 color;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;


void main()
{
    color = texture(diffuseTex, Texcoord0);
}
