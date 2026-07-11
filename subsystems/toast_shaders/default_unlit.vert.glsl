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

struct Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout (descriptor_heap) uniform CameraHeap
{
    Camera data;
} cameraHeap[];

layout (push_constant) uniform TSTC__Constants
{
    mat4 meshTransform;

    VertexBuffer vbo;

    uint _unused0[2];

    uint materialIndex;
    uint cameraIndex;

    uint _unused1[6];
} pcs;

layout (location = 0) out vec3 o_WorldPos;
layout (location = 1) out vec2 o_TexCoord;

void main()
{
    Vertex v_in = pcs.vbo.vertices[gl_VertexIndex];

    Camera camera = cameraHeap[nonuniformEXT(pcs.cameraIndex)].data;

    vec4 world_pos = pcs.meshTransform * v_in.position;
    vec4 view_pos = camera.view * world_pos;

    gl_Position = camera.proj * view_pos;

    o_WorldPos = world_pos.xyz;

    o_TexCoord = v_in.texCoord;
}
