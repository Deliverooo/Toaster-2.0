#version 460

#define VEC3_SIZE 12
#define UINT_SIZE 4
#define PI 3.1415926535f

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Position;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec3 v_Normal;
layout(location = 4) in mat3 v_WorldNormals;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_AlbedoTexture;
layout(set = 0, binding = 1) uniform sampler2D u_NormalTexture;

layout (set = 2, binding = 0) uniform samplerCube u_DiffuseIrradianceMap;
layout (set = 2, binding = 1) uniform sampler2D u_AOTexture;

layout(push_constant) uniform Material
{
    layout(offset = 64) vec3 albedoColour;
    layout(offset = 76) uint hasNormalMap;
    layout(offset = 80) float roughness;
    layout(offset = 84) float metalness;
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
    vec4 cameraPos;
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

    for (uint i = 0; i < u_DirectionalLights.count; i++)
    {
        DirectionalLight light = u_DirectionalLights.lights[i];

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

    for (uint i = 0; i < u_PointLights.count; i++)
    {
        PointLight light = u_PointLights.lights[i];

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
    if (u_Material.hasNormalMap != 0u)
    {
        params.normal = normalize(texture(u_NormalTexture, v_TexCoord).rgb * 2.0f - 1.0f);
        params.normal = normalize(v_WorldNormals * params.normal);
    }

    params.view = normalize(u_SceneData.cameraPos.xyz - v_WorldPos);// Get the direction of the view from the camera to the frag pos
    params.nDotV = max(dot(params.normal, params.view), 0.0f);// Tells us how much the view direction is aligned with the surface normal

    vec4 albedo_texture_colour = texture(u_AlbedoTexture, v_TexCoord);
    params.albedo = albedo_texture_colour.rgb * u_Material.albedoColour;

    params.metalness = u_Material.metalness;
    params.roughness = u_Material.roughness;

    params.F0 = vec3(0.04f);
    params.F0 = mix(params.F0, params.albedo, params.metalness);

    vec3 lo = vec3(0.0f);// Outgoing light

    lo += calcDirectionalLights(v_WorldPos);
    lo += calcPointLights(v_WorldPos);

    vec3 ks = fresnelSchlick(params.nDotV);
    vec3 kd = vec3(1.0f) - ks;
    kd *= 1.0f - params.metalness;

    vec3 irradiance = texture(u_DiffuseIrradianceMap, params.normal).rgb;
    vec3 diffuse_ambient = kd * irradiance * params.albedo;

    vec3 ambient = diffuse_ambient;

    vec2 screen_uv = gl_FragCoord.xy / vec2(textureSize(u_AOTexture, 0));
    float ao = texture(u_AOTexture, screen_uv).r;
    ambient *= ao;

    vec3 final_colour = ambient + lo;

    o_Colour = vec4(final_colour, 1.0f);
}