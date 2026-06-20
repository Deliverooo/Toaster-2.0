#version 460

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(location = 0) in vec3 m_Normal;
layout(location = 1) in vec2 m_TexCoord;
layout(location = 2) in flat uint m_SubmeshIndex;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform textureCube globalCubeMaps[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(push_constant) uniform Constants
{
    uint64_t vertexBuffer;
    uint64_t meshletBuffer;
    uint64_t meshletVertexIndexBuffer;
    uint64_t meshletTriangleIndexBuffer;

    uint64_t camera;

    uint samplerIndex;
    uint diffuseIrradianceMapIndex;
} pcs;

struct PBRGlobals
{
    vec3 normal;
    vec3 view;
    float nDotV;

    vec3 F0;
    vec3 albedo;
    float roughness;
    float metalness;
} glob;


void main()
{
    glob.normal = normalize(m_Normal);
//    glob.view = normalize(u_SceneData.cameraPos.xyz - v_WorldPos);// Get the direction of the view from the camera to the frag pos
//    glob.nDotV = max(dot(glob.normal, glob.view), 0.0001f);// Tells us how much the view direction is aligned with the surface normal

    vec4 environment_colour = textureLod(samplerCube(globalCubeMaps[pcs.diffuseIrradianceMapIndex], globalSamplers[pcs.samplerIndex]), glob.normal, 0);

    o_Colour = vec4(environment_colour.rgb, 1.0f);
}