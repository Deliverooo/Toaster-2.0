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

layout (buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout (push_constant) uniform Constants
{
    VertexBuffer vbo;
    Camera cameraPtr;

    uint samplerId;
    uint skyboxMapId;
} pcs;

layout (location = 0) out vec3 o_Pos;
layout (location = 1) out vec2 o_TexCoord;

void main()
{
    Vertex v_in = pcs.vbo.vertices[gl_VertexIndex];

    vec4 world_pos = vec4(v_in.position, 0.0f, 1.0f);
    gl_Position = world_pos;

    mat4 inv_view = mat4(mat3(inverse(pcs.cameraPtr.view)));

    o_Pos = (inv_view * pcs.cameraPtr.invProj * world_pos).xyz;
}
