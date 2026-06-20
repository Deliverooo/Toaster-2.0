#version 460
#extension GL_GOOGLE_include_directive : require
#include "common.glslh"

layout(location = 0) in vec2 m_TexCoord;

layout(location = 0) out vec4 o_Colour;


layout(push_constant) uniform PushConstants
{
    uint samplerIndex;
    uint textureIndex;
} pcs;

void main()
{
    vec4 texture_colour = texture(sampler2D(texture2DHeap[pcs.textureIndex], samplerHeap[pcs.samplerIndex]), m_TexCoord);
    o_Colour = texture_colour;
}