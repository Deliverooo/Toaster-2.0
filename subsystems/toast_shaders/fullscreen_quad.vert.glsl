#version 460
#extension GL_EXT_buffer_reference: require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

struct Vertex
{
    vec2 position;
    vec2 texCoord;
};

layout (std430, buffer_reference) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform Constants
{
    VertexBuffer vbo;

    uint samplerId;
    uint textureId;
} pcs;

layout (location = 0) out vec2 o_Pos;
layout (location = 1) out vec2 o_TexCoord;

void main()
{
    Vertex v_in = pcs.vbo.vertices[gl_VertexIndex];

    vec4 world_pos = vec4(v_in.position, 0.0f, 1.0f);
    gl_Position = world_pos;

    o_Pos = v_in.position;
    o_TexCoord = v_in.texCoord;
}
