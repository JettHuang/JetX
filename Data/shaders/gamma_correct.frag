#version 330 core

in vec3 fcolor;
uniform bool bGamma;

out vec4 color;

void main()
{
	if(bGamma)
	{
		color = vec4(pow(fcolor, vec3(1.f/2.2f)), 1.f);
	}
	else 
	{
		color = vec4(fcolor,1.f);
	}
}
