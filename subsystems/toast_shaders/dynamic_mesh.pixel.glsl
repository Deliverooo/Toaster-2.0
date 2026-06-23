#version 460
#extension GL_GOOGLE_include_directive : require
#include "pbr_common.glslh"

layout(location = 0) in vec3 m_WorldPos;
layout(location = 1) in vec3 m_Normal;
layout(location = 2) in vec2 m_TexCoord;
layout(location = 3) in flat uint m_MaterialIndex;

layout(location = 0) out vec4 o_Colour;

struct Material
{
    uint samplerIndex;
    uint albedoMapIndex;
    float _padd[2];

    vec4 albedoColour;

};

layout(buffer_reference, std140) readonly buffer SceneData { vec4 cameraPosition; };
layout(std430, buffer_reference) readonly buffer MaterialBuffer { Material materials[]; };

layout(push_constant) uniform Constants
{
    uint64_t vertexBuffer;
    uint64_t meshletBuffer;
    uint64_t meshletVertexIndexBuffer;
    uint64_t meshletTriangleIndexBuffer;

    mat4           meshTransform;
    uint64_t       submeshBuffer;
    MaterialBuffer materialBuffer;

    uint64_t camera;
    SceneData sceneData;

    uint samplerIndex;
    uint diffuseIrradianceMapIndex;
} pcs;

void main()
{
    glob.normal = normalize(m_Normal);

    glob.view = normalize(pcs.sceneData.cameraPosition.xyz - m_WorldPos);// Get the direction of the view from the camera to the frag pos
    glob.nDotV = max(dot(glob.normal, glob.view), 0.0001f);// Tells us how much the view direction is aligned with the surface normal

    Material material = pcs.materialBuffer.materials[m_MaterialIndex];

    glob.albedo = texture(sampler2D(texture2DHeap[material.albedoMapIndex], samplerHeap[material.samplerIndex]), m_TexCoord).rgb;// Temp
    //    glob.albedo = material.albedoColour.rgb;// Temp
    //    glob.albedo = vec3(1.0f);// Temp
    glob.metalness = 0.0f;// Temp
    glob.roughness = 0.5f;// Temp

    glob.f0 = vec3(0.04f);
    glob.f0 = mix(glob.f0, glob.albedo, glob.metalness);

    vec3 ks = fresnelSchlickRoughness(glob.nDotV, glob.f0, glob.roughness);
    vec3 kd = vec3(1.0f) - ks;
    kd *= 1.0f - glob.metalness;

    vec3 irradiance = textureLod(samplerCube(textureCubeHeap[pcs.diffuseIrradianceMapIndex], samplerHeap[pcs.samplerIndex]), glob.normal, 0).rgb;
    irradiance /= irradiance + vec3(1.0f);

    vec3 diffuse_ambient = kd * irradiance * glob.albedo;

    vec3 ambient = diffuse_ambient;

    vec3 final_colour = ambient;// + lo

    o_Colour = vec4(final_colour, 1.0f);
    //    o_Colour = vec4(final_colour, 1.0f);
}