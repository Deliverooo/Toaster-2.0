#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : require

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];


layout(buffer_reference, std140) readonly buffer UBO
{ 
    vec4 colourData;
};


layout(push_constant) uniform PushConstants
{
    uint textureIndex;
    uint samplerIndex;

    UBO currentUBOPtr;
} pcs;

void main()
{
    vec4 texture_colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), v_TexCoord);



    o_Colour = pcs.currentUBOPtr.colourData * texture_colour;
}