#version 460

#define PI 3.1415926535f
#define EPSILON 0.00001f

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Colour;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec3 v_Normal;

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out vec4 o_Position;
layout(location = 2) out vec4 o_Normal;

layout(set = 0, binding = 0) uniform sampler2D u_AlbedoTexture;

layout(push_constant) uniform Material
{
    layout(offset = 64) vec3 albedoColour;
} u_Material;

struct DirectionalLight
{
    vec4 direction;
    vec3 radiance;
    float multiplier;
};

layout(set = 1, binding = 2) uniform DirectionalLightData
{
    uint count;
    DirectionalLight lights[4];
} u_DirectionalLights;

struct PointLight
{
    vec4 position;
    vec3 radiance;
    float multiplier;
};

layout(set = 1, binding = 3) uniform PointLightData
{
    uint count;
    PointLight lights[128];
} u_PointLights;

layout(set = 1, binding = 4) uniform SceneData
{
    vec3 cameraPos;
} u_SceneData;

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


// ----------------------------------------------------------------------------
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

vec3 calcDirectionalLights()
{
    vec3 result = vec3(0.0f);

    for (uint i = 0; i < u_DirectionalLights.count; i++)
    {
        DirectionalLight light = u_DirectionalLights.lights[i];

        vec3 dir = -light.direction.xyz;
        vec3 L = normalize(dir);
        vec3 H = normalize(params.view + L);

        float distance = length(dir);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light.radiance * light.multiplier* attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(params.normal, H, params.roughness);
        float G   = GeometrySmith(params.normal, params.view, L, params.roughness);
        vec3 F    = fresnelSchlick(max(dot(H, params.view), 0.0), params.F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(params.normal, params.view), 0.0) * max(dot(params.normal, L), 0.0) + 0.0001;// + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - params.metalness;

        // scale light by NdotL
        float NdotL = max(dot(params.normal, L), 0.0);

        // add to outgoing radiance Lo
        result += (kD * params.albedo / PI + specular) * radiance * NdotL;// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    }

    return result;
}

vec3 calcPointLights()
{
    vec3 result = vec3(0.0f);

    for (uint i = 0; i < u_PointLights.count; i++)
    {
        PointLight light = u_PointLights.lights[i];

        vec3 L = normalize(light.position.xyz - v_WorldPos);
        vec3 H = normalize(params.view + L);

        float distance = length(light.position.xyz - v_WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light.radiance * light.multiplier * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(params.normal, H, params.roughness);
        float G   = GeometrySmith(params.normal, params.view, L, params.roughness);
        vec3 F    = fresnelSchlick(max(dot(H, params.view), 0.0), params.F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(params.normal, params.view), 0.0) * max(dot(params.normal, L), 0.0) + 0.0001;// + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - params.metalness;

        // scale light by NdotL
        float NdotL = max(dot(params.normal, L), 0.0);

        // add to outgoing radiance Lo
        result += (kD * params.albedo / PI + specular) * radiance * NdotL;// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    return result;
}

void main()
{
    o_Position = vec4(v_WorldPos, 1.0f);
    o_Normal   = vec4(normalize(v_Normal), 1.0f);

    vec4 albedo_texture_colour = texture(u_AlbedoTexture, v_TexCoord);
    params.albedo = albedo_texture_colour.rgb * u_Material.albedoColour;

    params.metalness = 0.0f;

    params.F0 = mix(vec3(0.04f), params.albedo, params.metalness);

    params.roughness = 0.5f;
    params.metalness = 0.0f;

    params.normal = normalize(v_Normal);

    params.view = normalize(u_SceneData.cameraPos - v_WorldPos);
    params.nDotV = max(dot(params.normal, params.view), 0.0);

    vec3 final_colour = vec3(params.albedo) * 0.02f;

    final_colour += calcDirectionalLights();
    final_colour += calcPointLights();

    o_Colour = vec4(final_colour, 1.0f);
}