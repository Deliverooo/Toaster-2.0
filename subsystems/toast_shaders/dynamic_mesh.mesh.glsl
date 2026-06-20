#version 460
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define MESH_SHADER_DISPATCH_SIZE 64

struct Vertex
{
    vec4 position;
    vec3 normal;
    float _padd[1];
    vec2 texCoord;
    float _padd2[2];
};

struct Meshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;

    uint submeshIndex;

    float _padd[3];
};

layout(local_size_x = MESH_SHADER_DISPATCH_SIZE, local_size_y = 1, local_size_z = 1) in;

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
    VertexBuffer               vertexBuffer;
    MeshletBuffer              meshletBuffer;
    MeshletVertexIndexBuffer   meshletVertexIndexBuffer;
    MeshletTriangleIndexBuffer meshletTriangleIndexBuffer;

    Camera                     camera;

    uint                       samplerIndex;
    uint                       diffuseIrradianceMapIndex;
} pcs;

layout(triangles, max_vertices = 64, max_primitives = 126) out;

layout(location = 0) out vec3 o_Normal[];
layout(location = 1) out vec2 o_TexCoord[];
layout(location = 2) out flat uint o_SubmeshIndex[];

void main()
{
    uint meshlet_index = gl_WorkGroupID.x;

    Meshlet m = pcs.meshletBuffer.meshlets[meshlet_index];

    SetMeshOutputsEXT(m.vertexCount, m.triangleCount);

    uint thread_id = gl_LocalInvocationID.x;

    for (uint i = thread_id; i < m.vertexCount; i += MESH_SHADER_DISPATCH_SIZE)
    {
        uint global_index = pcs.meshletVertexIndexBuffer.meshletVertices[m.vertexOffset + i];

        vec4 pos = pcs.vertexBuffer.vertices[global_index].position;
        gl_MeshVerticesEXT[i].gl_Position = pcs.camera.proj * pcs.camera.view * pos;

        vec2 tex_coord = pcs.vertexBuffer.vertices[global_index].texCoord.xy;
        tex_coord.y *= -1.0f;
        o_TexCoord[i] = tex_coord;

        o_Normal[i] = pcs.vertexBuffer.vertices[global_index].normal.xyz;

        o_SubmeshIndex[i] = m.submeshIndex;
    }

    for (uint i = thread_id; i < m.triangleCount; i += MESH_SHADER_DISPATCH_SIZE)
    {
        uint offset = m.triangleOffset + (i * 3u);

        uvec3 indices = uvec3(
        uint(pcs.meshletTriangleIndexBuffer.meshletTriangles[offset + 0u]),
        uint(pcs.meshletTriangleIndexBuffer.meshletTriangles[offset + 1u]),
        uint(pcs.meshletTriangleIndexBuffer.meshletTriangles[offset + 2u])
        );

        gl_PrimitiveTriangleIndicesEXT[i] = indices;
    }
}
