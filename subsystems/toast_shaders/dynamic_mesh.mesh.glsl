#version 460
#extension GL_EXT_mesh_shader : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_shader_8bit_storage : enable

#define MESH_SHADER_DISPATCH_SIZE 32

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

layout(local_size_x = MESH_SHADER_DISPATCH_SIZE, local_size_y = 1, local_size_z = 1) in;

// Input
layout(std430, buffer_reference) readonly buffer VertexBuffer { Vertex vertices[]; };
layout(std430, buffer_reference) readonly buffer MeshletBuffer { Meshlet meshlets[]; };
layout(std430, buffer_reference) readonly buffer MeshletVertexIndexBuffer { uint meshletVertices[]; };
layout(std430, buffer_reference) readonly buffer MeshletTriangleIndexBuffer { uint8_t meshletTriangles[]; };

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

layout(triangles, max_vertices = 64, max_primitives = 124) out;

layout(location = 0) out vec2 o_TexCoord[];

void main()
{
    uint meshlet_index = gl_WorkGroupID.x;
    Meshlet m = pcs.meshletBuffer.meshlets[meshlet_index];

    SetMeshOutputsEXT(m.vertexCount, m.triangleCount);
    uint thread_id = gl_LocalInvocationID.x;

    for (uint i = thread_id; i < m.vertexCount; i += MESH_SHADER_DISPATCH_SIZE)
    {
        uint global_index = pcs.meshletVertexIndexBuffer.meshletVertices[m.vertexOffset + i];
        gl_MeshVerticesEXT[i].gl_Position = pcs.camera.proj * pcs.camera.view * vec4(pcs.vertexBuffer.vertices[global_index].position, 1.0f);

        o_TexCoord[i] = pcs.vertexBuffer.vertices[global_index].texCoord;
    }

    for (uint i = thread_id; i < m.triangleCount; i += MESH_SHADER_DISPATCH_SIZE)
    {
        uint offset = m.triangleOffset + (i * 3);

        uvec3 indices = uvec3(
        pcs.meshletTriangleIndexBuffer.meshletTriangles[offset + 0],
        pcs.meshletTriangleIndexBuffer.meshletTriangles[offset + 1],
        pcs.meshletTriangleIndexBuffer.meshletTriangles[offset + 2]);

        gl_PrimitiveTriangleIndicesEXT[i] = indices;
    }

}
