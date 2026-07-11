#version 460
#extension GL_GOOGLE_include_directive: require
#include "pbr_common.glslh"

layout (location = 0) in vec3 v_WorldPos;
layout (location = 1) in vec2 v_TexCoord;

layout (location = 0) out vec4 o_Colour;

layout (descriptor_heap) uniform TST__MaterialUnlit
{
    uint textureSampler;
    uint albedoMap;

    vec4 albedoColour;
} materialHeap[];
#define MaterialUnlit TST__MaterialUnlit

layout (push_constant) uniform TSTC__Constants
{
    mat4 meshTransform;

    uint64_t vbo;

    uint _unused0[2];

    uint materialIndex;
    uint cameraIndex;

    uint _unused1[6];
} pcs;

void main()
{
    glob.albedo = vec4(1.0f);
    glob.albedo = texture(sampler2D(texture2DHeap[materialHeap[nonuniformEXT(pcs.materialIndex)].albedoMap], samplerHeap[materialHeap[nonuniformEXT(pcs.materialIndex)].textureSampler]), v_TexCoord);
    glob.albedo.rgb *= materialHeap[nonuniformEXT(pcs.materialIndex)].albedoColour.rgb;

    if (glob.albedo.a < 1.0f)
    {
        glob.albedo = vec4(1.0f, 0.0f, 1.0f, 1.0f);
    }

    vec3 final_colour = glob.albedo.rgb;

    o_Colour = vec4(final_colour, 1.0f);
}