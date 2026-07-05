#version 460
#extension GL_GOOGLE_include_directive : require
#include "common.glslh"

layout(location = 0) in vec2 v_Position;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(push_constant) uniform PushConstants
{
    uint64_t vertexBuffer;

    uint samplerIndex;
    uint textureIndex;
} pcs;

void main()
{
    vec4 texture_colour = texture(sampler2D(texture2DHeap[pcs.textureIndex], samplerHeap[pcs.samplerIndex]), v_TexCoord);
    o_Colour = texture_colour;
}