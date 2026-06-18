#version 460
#extension GL_EXT_mesh_shader : require

layout(local_size_x = 3, local_size_y = 1, local_size_z = 1) in;

layout(triangles, max_vertices = 3, max_primitives = 3) out;

layout(location = 0) out vec3 outColor[];

const vec3 positions[3] = vec3[](
vec3(0.0, -0.5, 0.0),
vec3(0.5, 0.5, 0.0),
vec3(-0.5, 0.5, 0.0)
);

const vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0), // Red
vec3(0.0, 1.0, 0.0), // Green
vec3(0.0, 0.0, 1.0)// Blue
);

void main()
{
    uint iID = gl_LocalInvocationID.x;

    if (iID == 0)
    {
        SetMeshOutputsEXT(3, 1);
        barrier();
    }

    if (iID < 3)
    {
        gl_MeshVerticesEXT[iID].gl_Position = vec4(positions[iID], 1.0);
        outColor[iID] = colors[iID];
    }
    if (iID == 0)
    {
        gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
    }
}
