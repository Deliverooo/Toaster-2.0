#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec2 m_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoord;
};

struct Meshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
};

// Input
layout(std430, buffer_reference) readonly buffer VertexBuffer { Vertex vertices[]; };
layout(std430, buffer_reference) readonly buffer MeshletBuffer { Meshlet meshlets[]; };
layout(std430, buffer_reference) readonly buffer MeshletVertexIndexBuffer { uint meshletVertices[]; };
layout(std430, buffer_reference) readonly buffer MeshletTriangleIndexBuffer { uint meshletTriangles[]; };

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
};

layout(push_constant) uniform Constants
{
    VertexBuffer vertexBuffer;
    MeshletBuffer meshletBuffer;
    MeshletVertexIndexBuffer meshletVertexIndexBuffer;
    MeshletTriangleIndexBuffer meshletTriangleIndexBuffer;

    Camera camera;

    uint samplerIndex;
    uint textureIndex;
} pcs;


void main()
{
    vec4 texture_colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), m_TexCoord);

    o_Colour =texture_colour;
}