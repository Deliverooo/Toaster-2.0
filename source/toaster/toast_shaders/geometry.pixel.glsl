#version 460
#define PI 3.1415926535f
#define EPSILON 0.00001f

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Colour;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec3 v_Normal;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_AlbedoTexture;

layout(push_constant) uniform Material
{
    layout(offset = 64) vec3 albedoColour;
} u_Material;

struct PointLight
{
    vec3 position;
    vec3 radiance;

    float radius;
    float falloff;
    float multiplier;
};

layout(set = 1, binding = 2) uniform PointLightData
{
    uint count;
    PointLight lights[128];
} u_PointLights;

layout(set = 1, binding = 3) uniform SceneData
{
    vec3 cameraPos;
} u_SceneData;

// Ts is just usefull to have so we can reference it in multiple functions :)
struct PBRGlobals
{
    vec3 albedo;
    float roughness;
    float metalness;

    vec3 normal;
    vec3 view;
    float nDotV;
} params;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec4 albedo_texture_colour = texture(u_AlbedoTexture, v_TexCoord);
    params.albedo = albedo_texture_colour.rgb * u_Material.albedoColour;
    params.roughness = 1.0f;
    params.metalness = 0.0f;

    params.normal = normalize(v_Normal);

    params.view = normalize(u_SceneData.cameraPos - v_Position);
    params.nDotV = max(dot(params.normal, params.view), 0.0);

    vec3 light_contribution = params.albedo;

    vec3 ambient = params.albedo * 0.2f;

    //    for (uint i = 0; i < u_PointLights.count; i++)
    //    {
    //        PointLight point_light = u_PointLights.lights[i];
    //
    //        float light_distance = length(point_light.position - v_Position);
    //
    //        float attenuation = clamp(1.0f - (light_distance * light_distance) / (point_light.radius * point_light.radius), 0.0, 1.0);
    //        attenuation *= mix(attenuation, 1.0f, point_light.falloff);
    //
    //        vec3 light_radiance = point_light.radiance * 10.0f * attenuation;
    //
    //        vec3 unitLightDirection = normalize(point_light.position - v_Position);
    //        vec3 specularReflectDirection = reflect(-unitLightDirection, params.normal);
    //
    //        float diff = max(dot(params.normal, unitLightDirection), 0.0f);
    //        float spec = pow(max(dot(params.view, specularReflectDirection), 0.0f), params.roughness);
    //
    //        vec3 diffuse  = light_radiance* diff * params.albedo;
    //        vec3 specular = spec * vec3(1.0f) * 0.3f;
    //
    //        specular *= attenuation;
    //
    //        light_contribution += (diffuse + specular + ambient);
    //}

    o_Colour = vec4(light_contribution, 1.0f);
}