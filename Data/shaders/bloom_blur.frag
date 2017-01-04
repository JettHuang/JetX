#version 330 core

out vec4 FragColor;
in vec2 Texcoord;

uniform sampler2D  quadTex;
uniform bool bHorizontal;

uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
	vec2 texSize = textureSize(quadTex, 0);
	vec2 texstep = 1.0 / texSize;

	vec3 result = texture(quadTex, Texcoord).rgb * weight[0];
	if(bHorizontal)
	{
		for(int i=1; i<5; i++)
		{
			result += texture(quadTex, Texcoord + vec2(texstep.x * i, 0.0)).rgb * weight[i];
			result += texture(quadTex, Texcoord - vec2(texstep.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for(int i=1; i<5; i++)
		{
			result += texture(quadTex, Texcoord + vec2(0.0, texstep.y * i)).rgb * weight[i];
			result += texture(quadTex, Texcoord - vec2(0.0, texstep.y * i)).rgb * weight[i];
		}
	}

	FragColor = vec4(result, 1.0);
}
