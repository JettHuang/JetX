#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 shadowClipPos;
} fs_in;


uniform sampler2D  diffuseTex;
uniform sampler2D  shadowMapTex;
uniform vec3 lightPos;
uniform vec3 viewPos;


float calculateShadowFactor(vec4 projFragPos)
{
	// map to [0, 1]
	vec3 pos = projFragPos.xyz / projFragPos.w;
	pos = pos * 0.5 + 0.5;
	float depthInShadowMap = texture(shadowMapTex, pos.xy).r;

	float factor = 0.0;
	float thisDepth = pos.z - 0.005f; 
	if(thisDepth > depthInShadowMap)
	{
		factor = 0.5;
	}
	//if(pos.z > 1.0)
	//{
	//	factor = 0.0;
	//}

	return factor;
}

void main()
{
	vec3 color = texture(diffuseTex, fs_in.TexCoords).rgb;
	vec3 ambient = 0.05 * color;

	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;

	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec;

	FragColor = vec4(ambient + diffuse + specular, 1.0f);

	float shadowFactor = calculateShadowFactor(fs_in.shadowClipPos);
	FragColor = mix(FragColor, vec4(0.f,0.f,0.f,1.f), shadowFactor);
}
