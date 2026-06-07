#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;


layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(push_constant) uniform PushConstants
{
    uint textureIndex;
    uint samplerIndex;
} pcs;

void main()
{
    o_Colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), v_TexCoord);
}