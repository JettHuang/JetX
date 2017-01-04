#version 330 core

in vec2  Texcoord;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
	vec3 Position;
	vec3 Color;

	float Linear;
	float Quadratic;
	float Radius;
}; 

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

uniform int draw_mode;


void main()
{
    // Retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, Texcoord).rgb;
    vec3 Normal = texture(gNormal, Texcoord).rgb;
	vec4 albedoColor = texture(gAlbedoSpec, Texcoord);
    vec3 Diffuse = albedoColor.rgb;
    float Specular = albedoColor.a;

	// Then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Calculate distance between light source and current fragment
        float distance = length(lights[i].Position - FragPos);
        if(distance < lights[i].Radius)
        {
            // Diffuse
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // Specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;
            // Attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }


	// Based on which of the 1-5 keys we pressed, show final result or intermediate g-buffer textures
    if(draw_mode == 1)
        FragColor = vec4(lighting, 1.0);
    else if(draw_mode == 2)
        FragColor = vec4(FragPos, 1.0);
    else if(draw_mode == 3)
        FragColor = vec4(Normal, 1.0);
    else if(draw_mode == 4)
        FragColor = vec4(Diffuse, 1.0);
    else if(draw_mode == 5)
        FragColor = vec4(vec3(Specular), 1.0);
}
