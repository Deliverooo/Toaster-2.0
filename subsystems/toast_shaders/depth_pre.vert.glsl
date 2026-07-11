#version 460
#extension GL_GOOGLE_include_directive: require
#include "common.glslh"

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
//layout (std430, buffer_reference, scalar) readonly buffer IndexBuffer { uint indices[]; };

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
//    IndexBuffer ibo;

    Camera cameraPtr;
} pcs;

void main()
{
//    uint index = pcs.ibo.indices[gl_VertexIndex + pcs.indexOffset];
//    Vertex v_in = pcs.vbo.vertices[index + pcs.vertexOffset];
    Vertex v_in = pcs.vbo.vertices[gl_VertexIndex];

    vec4 world_pos = pcs.meshTransform * v_in.position;
    vec4 view_pos = pcs.cameraPtr.view * world_pos;

    gl_Position = pcs.cameraPtr.proj * view_pos;
}