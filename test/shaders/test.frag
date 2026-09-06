#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout (location = 0) in vec2 v_TexCoord;
layout (location = 0) out vec4 o_Colour;

layout (descriptor_heap) uniform texture2D texture2DHeap[];
layout (descriptor_heap) uniform sampler samplerHeap[];

layout (push_constant) uniform PushData
{
    uint textureHeapSlot;
    uint samplerHeapSlot;
} pcs;

void main()
{
    vec4 tex_colour = texture(sampler2D(texture2DHeap[pcs.textureHeapSlot], samplerHeap[pcs.samplerHeapSlot]), v_TexCoord);

    o_Colour = vec4(tex_colour.rgb, 1.0f);
}