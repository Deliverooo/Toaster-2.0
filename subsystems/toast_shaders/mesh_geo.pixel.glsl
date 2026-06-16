#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

#define PI 3.1415926535f

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in mat3 v_WorldNormals;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

struct DirectionalLight
{
    vec4 direction;
    vec3 radiance;
    float multiplier;
};

layout(buffer_reference, std140) readonly buffer DirectionalLights
{
    uint count;
    DirectionalLight lights[4];
};

struct PointLight
{
    vec4 position;
    vec3 radiance;
    float multiplier;
};

layout(buffer_reference, std140) readonly buffer PointLights
{
    uint count;
    PointLight lights[128];
};

layout(buffer_reference, std140) readonly buffer SceneData
{
    vec3 cameraPos;
    float _padd;
};

layout(push_constant) uniform PushConstants
{
    Camera camera;
    DirectionalLights directionalLights;
    PointLights pointLights;
    SceneData sceneData;

    mat4 modelMatrix;

    vec4 albedoColour;

    uint samplerIndex;

    uint albedoMap;
    uint normalMap;
    uint hasNormalMap;

    float roughness;
    float metalness;

    float _padd[2];
} pcs;

// Ts is just usefull to have so we can reference it in multiple functions :)
struct PBRGlobals
{
    vec3 F0;
    vec3 albedo;
    float roughness;
    float metalness;

    vec3 normal;
    vec3 view;
    float nDotV;
} params;


float distributionGGX(vec3 p_halfway, float p_roughness)
{
    float a = p_roughness * p_roughness;
    float a_squared = a * a;
    float ndoth = max(dot(params.normal, p_halfway), 0.0);
    float ndoth_squared = ndoth * ndoth;

    float numerator   = a_squared;
    float denominator = (ndoth_squared * (a_squared - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return numerator / denominator;
}

float geometrySchlickGGX(float p_ndotv, float p_roughness)
{
    float r = (p_roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float num   = p_ndotv;
    float denom = p_ndotv * (1.0f - k) + k;

    return num / denom;
}

float geometrySmith(float p_ndotl, float p_roughness)
{
    float ggx2 = geometrySchlickGGX(params.nDotV, p_roughness);
    float ggx1 = geometrySchlickGGX(p_ndotl, p_roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float p_cos_theta)
{
    return params.F0 + (1.0f - params.F0) * pow(clamp(1.0f - p_cos_theta, 0.0f, 1.0f), 5.0f);
}


vec3 calcDirectionalLights(vec3 p_point)
{
    vec3 result = vec3(0.0f);

    for (uint i = 0; i < pcs.directionalLights.count; i++)
    {
        DirectionalLight light = pcs.directionalLights.lights[i];

        vec3 light_dir = normalize(-light.direction.xyz);// Also denoted wi
        vec3 halfway_dir = normalize(light_dir + params.view);// Halfway between the light's direction and the view direction
        float ndotl = max(dot(params.normal, light_dir), 0.0f);// Amount the light's direction lines up with the surface normal
        float hdotl = max(dot(halfway_dir, light_dir), 0.0f);

        vec3 light_radiance = light.radiance * light.multiplier;

        float normal_distribution = distributionGGX(halfway_dir, params.roughness);
        float geometry = geometrySmith(ndotl, params.roughness);
        vec3 fresnel = fresnelSchlick(hdotl);
        vec3 dfg = normal_distribution * geometry * fresnel;

        float denom = max(4.0f * params.nDotV * ndotl, 0.0001f);// Dont divide by 0...

        vec3 ks = fresnel;
        vec3 kd = vec3(1.0f) - ks;
        kd *= 1.0f - params.metalness;

        result += ((kd * (params.albedo / PI)) + (dfg / denom)) * light_radiance * ndotl;
    }

    return result;
}

vec3 calcPointLights(vec3 p_point)
{
    vec3 result = vec3(0.0f);

    for (uint i = 0; i < pcs.pointLights.count; i++)
    {
        PointLight light = pcs.pointLights.lights[i];

        vec3 light_dir = normalize(light.position.xyz - p_point);// Also denoted wi
        vec3 halfway_dir = normalize(light_dir + params.view);// Halfway between the light's direction and the view direction
        float ndotl = max(dot(params.normal, light_dir), 0.0f);// Amount the light's direction lines up with the surface normal
        float hdotl = max(dot(halfway_dir, light_dir), 0.0f);

        float distance = length(light.position.xyz - p_point);// Distance from the light to the frag pos
        float attenuation = 1.0f / (distance * distance);// Inverse square law
        vec3 light_radiance = light.radiance * light.multiplier * attenuation;

        float normal_distribution = distributionGGX(halfway_dir, params.roughness);
        float geometry = geometrySmith(ndotl, params.roughness);
        vec3 fresnel = fresnelSchlick(hdotl);
        vec3 dfg = normal_distribution * geometry * fresnel;

        float denom = max(4.0f * params.nDotV * ndotl, 0.0001f);// Dont divide by 0...

        vec3 ks = fresnel;
        vec3 kd = vec3(1.0f) - ks;
        kd *= 1.0f - params.metalness;

        result += ((kd * (params.albedo / PI)) + (dfg / denom)) * light_radiance * ndotl;
    }

    return result;
}


void main()
{
    params.normal = normalize(v_Normal);// We have to normalise our normal

    if (pcs.hasNormalMap != 0u)
    {
        params.normal = normalize(texture(sampler2D(globalTextures[pcs.normalMap], globalSamplers[pcs.samplerIndex]), v_TexCoord).rgb * 2.0f - 1.0f);
        params.normal = normalize(v_WorldNormals * params.normal);
    }

    params.view = normalize(pcs.sceneData.cameraPos.xyz - v_Position);// Get the direction of the view from the camera to the frag pos
    params.nDotV = max(dot(params.normal, params.view), 0.0001f);// Tells us how much the view direction is aligned with the surface normal
//
//    vec4 albedo_texture_colour = texture(sampler2D(globalTextures[pcs.albedoMap], globalSamplers[pcs.samplerIndex]), v_TexCoord);
//    params.albedo = albedo_texture_colour.rgb * pcs.albedoColour.rgb;
//
//    params.metalness = pcs.metalness;
//    params.roughness = pcs.roughness;
//
//    params.F0 = vec3(0.04f);
//    params.F0 = mix(params.F0, params.albedo, params.metalness);
//
//    vec3 lo = vec3(0.05f);// Outgoing light
//
//    lo += calcDirectionalLights(v_Position);
//    lo += calcPointLights(v_Position);
//
//    vec3 ks = fresnelSchlick(params.nDotV);
//    vec3 kd = vec3(1.0f) - ks;
//
//    kd *= 1.0f - params.metalness;
//    vec3 final_colour = lo;

    o_Colour = vec4(vec3( params.nDotV), 1.0f);
}
