#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec2 m_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];


layout(push_constant) uniform PushConstants
{
    uint samplerIndex;
    uint textureIndex;

} pcs;

void main()
{
    vec4 texture_colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), m_TexCoord);

    o_Colour =texture_colour;
}