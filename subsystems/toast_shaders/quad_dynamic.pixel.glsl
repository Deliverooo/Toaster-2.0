#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec4 v_Colour;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat uint v_TexIndex;
layout(location = 3) in float v_TilingFactor;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
};

layout(push_constant) uniform PushConstants
{
    Camera camera;

    uint samplerIndex;
    uint textureIndex;
} pcs;

void main()
{
    vec4 texture_colour = texture(sampler2D(globalTextures[v_TexIndex], globalSamplers[pcs.samplerIndex]), v_TexCoord * v_TilingFactor);
    o_Colour = vec4(texture_colour);
}