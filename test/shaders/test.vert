#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout (location = 0) out vec2 o_TexCoord;

const vec3 vertices[3u] = {
vec3(-1.0f, 1.0f, 0.0f),
vec3(1.0f, 1.0f, 0.0f),
vec3(0.0f, -1.0f, 0.0f)
};

const vec2 tex_coords[3u] = {
vec2(-1.0f, 1.0f),
vec2(1.0f, 1.0f),
vec2(0.0f, -1.0f)
};

layout (push_constant) uniform PushData
{
    uint textureHeapSlot;
    uint samplerHeapSlot;
} pcs;

void main()
{

    gl_Position = vec4(vertices[gl_VertexIndex], 1.0f);

    o_TexCoord = tex_coords[gl_VertexIndex];
}