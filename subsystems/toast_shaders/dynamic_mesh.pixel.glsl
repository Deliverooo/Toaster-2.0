#version 460
#extension GL_GOOGLE_include_directive : require
#include "pbr_common.glslh"

layout(location = 0) in vec3 m_WorldPos;
layout(location = 1) in vec3 m_Normal;
layout(location = 2) in vec2 m_TexCoord;
layout(location = 3) in flat uint m_SubmeshIndex;

layout(location = 0) out vec4 o_Colour;

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
};

layout(buffer_reference, std140) readonly buffer SceneData
{
    vec4 cameraPosition;
};

layout(push_constant) uniform Constants
{
    uint64_t vertexBuffer;
    uint64_t meshletBuffer;
    uint64_t meshletVertexIndexBuffer;
    uint64_t meshletTriangleIndexBuffer;

    Camera camera;
    SceneData sceneData;

    uint samplerIndex;
    uint diffuseIrradianceMapIndex;
} pcs;

void main()
{
    glob.normal = normalize(m_Normal);

    glob.view = normalize(pcs.sceneData.cameraPosition.xyz - m_WorldPos);// Get the direction of the view from the camera to the frag pos
    glob.nDotV = max(dot(glob.normal, glob.view), 0.0001f);// Tells us how much the view direction is aligned with the surface normal

    vec4 environment_colour = textureLod(samplerCube(textureCubeHeap[pcs.diffuseIrradianceMapIndex], samplerHeap[pcs.samplerIndex]), glob.normal, 0);

    o_Colour = vec4(vec3(glob.nDotV), 1.0f);
    //    o_Colour = vec4(environment_colour.rgb, 1.0f);
}