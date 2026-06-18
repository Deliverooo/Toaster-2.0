#version 460
#extension GL_EXT_mesh_shader : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(triangles, max_vertices = 4, max_primitives = 2) out;

layout(location = 0) out vec2 o_TexCoord[];

const vec4 positions[4] = vec4[4](
vec4(1.0f, 1.0f, 0.0f, 1.0f),
vec4(1.0f, -1.0f, 0.0f, 1.0f),
vec4(-1.0f, -1.0f, 0.0f, 1.0f),
vec4(-1.0f, 1.0f, 0.0f, 1.0f)
);

const vec2 uvs[4] = vec2[4](
vec2(1.0f, 1.0f),
vec2(1.0f, 0.0f),
vec2(0.0f, 0.0f),
vec2(0.0f, 1.0f)
);

void main()
{
    if (gl_LocalInvocationIndex == 0)
    {
        SetMeshOutputsEXT(4, 2);
    }

    for (uint i = 0u; i < 4u; ++i)
    {
        gl_MeshVerticesEXT[i].gl_Position = positions[i];
        o_TexCoord[i] = uvs[i];
    }

    gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 3);
    gl_PrimitiveTriangleIndicesEXT[1] = uvec3(1, 2, 3);
}
