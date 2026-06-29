#version 460
#extension GL_EXT_buffer_reference: require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

struct Vertex
{
    vec4 position;
    vec3 normal;
    float _padd1[1];
    vec3 tangent;
    float _padd2[1];
    vec3 bitangent;
    float _padd3[1];
    vec2 texCoord;
    float _padd4[2];
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
    mat4 meshTransform;

    VertexBuffer vbo;

    Camera cameraPtr;
} pcs;

void main()
{
    Vertex v_in = pcs.vbo.vertices[gl_VertexIndex];

    vec4 world_pos = pcs.meshTransform * v_in.position;
    vec4 view_pos = pcs.cameraPtr.view * world_pos;

    gl_Position = pcs.cameraPtr.proj * view_pos;
}