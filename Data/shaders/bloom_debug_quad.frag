#version 330 core

in vec2  Texcoord;

out vec4 color;

uniform sampler2D  quadTex;


void main()
{             
    color = texture(quadTex, Texcoord);
}
