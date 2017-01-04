#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord0;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 BoneIDs;
layout (location = 6) in vec4 Weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 gBones[75];

// varying outputs
out vec2   Texcoord0;


void main()
{
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

	vec4 local_pos = vec4(position, 1.0f);
	vec4 obj_pos = BoneTransform * local_pos;

    gl_Position = projection * view * model * obj_pos;
    Texcoord0 = texcoord0;
}

