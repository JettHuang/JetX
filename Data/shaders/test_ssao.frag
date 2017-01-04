#version 330 core

out float FragColor;

in vec2 Texcoord;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;


uniform vec3 samples[64]; // kernel sample points
const int kernelSize = 64;
const float radius = 1.0;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0);

uniform mat4 projection;


void main()
{
	vec3 fragPos = texture(gPositionDepth, Texcoord).xyz; // in view space
	vec3 normal = texture(gNormal, Texcoord).rgb;
	vec3 randomVec = texture(texNoise, Texcoord * noiseScale).xyz;

	// Create BTN change-of-basic matrix, from tangent-space to view-space
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// calculate the ambient occlusion factor
	float occlusion = 0.0;
	for(int i=0; i < kernelSize; ++i)
	{
		vec3 sample = TBN * samples[i]; // from tangent to view-space
		sample = fragPos + sample * radius;

		// project sample position
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 0.1

		// get sample depth
		float sampleDepth = texture(gPositionDepth, offset.xy).w;

		// range chech & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / kernelSize);

	FragColor = occlusion;
}
