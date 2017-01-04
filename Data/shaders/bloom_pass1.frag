#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

struct Light {
	vec3 Position;
	vec3 Color;
};

uniform Light light;
uniform sampler2D diffuseTex;
uniform vec3 viewPos;

void main()
{
	vec3 color = texture(diffuseTex, fs_in.TexCoords).rgb;

	// Ambient
	vec3 ambient = 0.3 * color;
	// Lighting
	vec3 result = vec3(0.0f);

	vec3 lightDir = normalize(light.Position - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;

	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec * light.Color;

	result = ambient + diffuse + specular;
	
	// Check whether result is higher than some threshold, if so, output as bloom threshold color
	float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 1.0)
		BrightColor = vec4(result, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	FragColor = vec4(result, 1.0);
}
