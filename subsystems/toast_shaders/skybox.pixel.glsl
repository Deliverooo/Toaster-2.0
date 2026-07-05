#version 460
#extension GL_GOOGLE_include_directive: require
#include "common.glslh"

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec2 v_TexCoord;

layout (location = 0) out vec4 o_Colour;

layout (buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout (push_constant) uniform PushConstants
{
    uint64_t vertexBuffer;
    Camera camera;

    uint samplerId;
    uint skyboxMapId;

} pcs;

void main()
{
    vec4 texture_colour = textureLod(samplerCube(textureCubeHeap[pcs.skyboxMapId], samplerHeap[pcs.samplerId]), normalize(v_Position), 1);
    o_Colour = vec4(texture_colour.rgb, 1.0f);
}