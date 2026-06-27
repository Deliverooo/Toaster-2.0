#version 460
#extension GL_GOOGLE_include_directive : require
#include "pbr_common.glslh"

layout(location = 0) in vec3 m_WorldPos;
layout(location = 1) in vec3 m_Normal;
layout(location = 2) in vec2 m_TexCoord;

layout(location = 0) out vec4 o_Colour;

struct TST__Material
{
    uint samplerIndex;
    uint albedoMapIndex;

    float roughness;
    float metalness;

    vec4 albedoColour;
};  

layout(buffer_reference, std140) readonly buffer SceneData { vec4 cameraPosition; };
layout(std430, buffer_reference) readonly buffer MaterialBuffer { TST__Material materials[]; };

layout(push_constant) uniform Constants
{
    uint64_t vbo;
    float _padd[2];

    mat4            meshTransform;
    MaterialBuffer  materialBuffer;
    uint            materialIndex;
    float           _padd2[1];

    uint64_t cameraPtr;
    SceneData sceneDataPtr;

    uint samplerIndex;
    uint diffuseIrradianceMapIndex;
} pcs;

void main()
{
    glob.normal = normalize(m_Normal);

    vec3 cam_pos = pcs.sceneDataPtr.cameraPosition.xyz;

    glob.view = normalize(cam_pos - m_WorldPos);// Get the direction of the view from the camera to the frag pos
    glob.nDotV = max(dot(glob.normal, glob.view), 0.0001f);// Tells us how much the view direction is aligned with the surface normal

    TST__Material material = pcs.materialBuffer.materials[pcs.materialIndex];

    glob.albedo = texture(sampler2D(texture2DHeap[material.albedoMapIndex], samplerHeap[material.samplerIndex]), m_TexCoord).rgb;
    glob.albedo *= material.albedoColour.rgb;

    glob.roughness = material.roughness;
    glob.metalness = material.metalness;

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

    //    o_Colour = vec4(glob.normal, 1.0f);
    //        o_Colour = vec4(glob.f0, 1.0f);
    //        o_Colour = vec4(vec3(glob.nDotV), 1.0f);
    o_Colour = vec4(final_colour, 1.0f);
}