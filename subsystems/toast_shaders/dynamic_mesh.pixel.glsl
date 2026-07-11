#version 460
#extension GL_GOOGLE_include_directive: require
#include "pbr_common.glslh"

layout (location = 0) in vec3 m_WorldPos;
layout (location = 1) in vec3 m_Normal;
layout (location = 2) in vec2 m_TexCoord;
layout (location = 3) in mat3 v_TBN;

layout (location = 0) out vec4 o_Colour;

struct Material
{
    uint textureSampler;
    uint albedoMap;

    float roughness;
    float metalness;

    vec4 albedoColour;
};

layout (std140, buffer_reference) readonly buffer TST__MaterialFairs
{
    uint textureSampler;
    uint albedoMap;
    uint hasNormalMap;
    uint normalMap;

    float roughness;
    float metalness;

    vec4 albedoColour;
};
#define MaterialFairs TST__MaterialFairs

layout (buffer_reference, std140) readonly buffer SceneData { vec4 cameraPosition; };

struct PointLight
{
    vec4 position;
    vec4 colourIntensity;
};
layout (buffer_reference, std430) readonly buffer PointLights { uint count; float _padd[3]; PointLight pointLights[128];};

layout (push_constant) uniform Constants
{
    mat4 meshTransform;

    uint64_t vbo;

    MaterialFairs material;

    uint64_t cameraPtr;
    SceneData sceneDataPtr;
    PointLights pointLightsPtr;

    uint samplerIndex;
    uint diffuseIrradianceMapIndex;
    uint specularIrradianceMapIndex;

    uint BRDFLUTSamplerIndex;
    uint BRDFLUT;
} pcs;

vec3 calcPointLights()
{
    vec3 result = vec3(0.0f);

    for (uint i = 0u; i < pcs.pointLightsPtr.count; ++i)
    {
        PointLight light = pcs.pointLightsPtr.pointLights[i];

        vec3 l = normalize(light.position.xyz - m_WorldPos);
        vec3 h = normalize(glob.view + l);

        float distance = length(light.position.xyz - m_WorldPos);
        float attenuation = 1.0f / (distance * distance);

        vec3 radiance = light.colourIntensity.xyz * vec3(light.colourIntensity.w) * attenuation;

        float ndf = distributionGGX(glob.normal, h, glob.roughness);

        float nDotL = max(dot(glob.normal, l), 0.0f);
        float g = geometrySmith(glob.nDotV, nDotL, glob.roughness);
        vec3 f = fresnelSchlick(glob.f0, max(dot(h, glob.view), 0.0f));

        vec3 ks = f;
        vec3 kd = vec3(1.0f) - ks;
        kd *= 1.0f - glob.metalness;

        vec3 numerator = ndf * g * f;
        float denominator = 4.0f * glob.nDotV * nDotL + 0.0001f;
        vec3 specular = numerator / denominator;

        result += (kd * glob.albedo.rgb / PI + specular) * radiance * nDotL;
    }


    return result;
}

void main()
{
    glob.normal = normalize(m_Normal);

    MaterialFairs material = pcs.material;

    if (material.hasNormalMap != 0u)
    {
        vec3 normal = texture(sampler2D(texture2DHeap[material.normalMap], samplerHeap[material.textureSampler]), m_TexCoord).xyz;
        normal = normalize(normal * 2.0f - 1.0f);
        glob.normal = normalize(v_TBN * normal);
    }

    vec3 cam_pos = pcs.sceneDataPtr.cameraPosition.xyz;

    glob.view = normalize(cam_pos - m_WorldPos); // Get the direction of the view from the camera to the frag pos
    glob.nDotV = max(dot(glob.normal, glob.view), 0.0001f); // Tells us how much the view direction is aligned with the surface normal

    glob.albedo = vec4(1.0f);
    glob.albedo = texture(sampler2D(texture2DHeap[material.albedoMap], samplerHeap[material.textureSampler]), m_TexCoord);
    glob.albedo.rgb *= material.albedoColour.rgb;

    if (glob.albedo.a < 1.0f)
    {
        glob.albedo = vec4(1.0f, 0.0f, 1.0f, 1.0f);
    }

    glob.roughness = material.roughness;
    glob.metalness = material.metalness;

    glob.f0 = vec3(0.04f);
    glob.f0 = mix(glob.f0, glob.albedo.rgb, glob.metalness);

    vec3 ks = fresnelSchlickRoughness(glob.nDotV, glob.f0, glob.roughness);
    vec3 kd = vec3(1.0f) - ks;
    kd *= 1.0f - glob.metalness;

    vec3 reflection_vec = reflect(-glob.view, glob.normal);

    float lod = glob.roughness * float(textureQueryLevels(samplerCube(textureCubeHeap[pcs.specularIrradianceMapIndex], samplerHeap[pcs.samplerIndex])) - 1);
    vec3 prefiltered_colour = textureLod(samplerCube(textureCubeHeap[pcs.specularIrradianceMapIndex], samplerHeap[pcs.samplerIndex]), reflection_vec, lod).rgb;

    vec2 brdfCoord = vec2(glob.nDotV, glob.roughness);
    vec2 brdf = texture(sampler2D(texture2DHeap[pcs.BRDFLUT], samplerHeap[pcs.BRDFLUTSamplerIndex]), brdfCoord).rg;

    vec3 specular_ambient = prefiltered_colour * (ks * brdf.x + brdf.y);

    vec3 diffuse_irradiance = textureLod(samplerCube(textureCubeHeap[pcs.diffuseIrradianceMapIndex], samplerHeap[pcs.samplerIndex]), glob.normal, 0).rgb;

    vec3 diffuse_ambient = kd * diffuse_irradiance * glob.albedo.rgb;

    vec3 ambient = diffuse_ambient;

    vec3 lo = vec3(0.0f);
    lo += calcPointLights();

    vec3 final_colour = ambient + lo;

    o_Colour = vec4(final_colour, 1.0f);
}