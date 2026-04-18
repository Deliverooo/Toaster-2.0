#version 460
#define PI 3.1415926535f
#define EPSILON 0.00001f

layout(location = 0) in vec3 v_Position;
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

void main()
{
    o_Position = vec4(v_Position, 1.0f);
    o_Normal   = vec4(normalize(v_Normal), 1.0f);

    vec4 albedo_texture_colour = texture(u_AlbedoTexture, v_TexCoord);
    params.albedo = albedo_texture_colour.rgb * u_Material.albedoColour;
    params.roughness = 1.0f;
    params.metalness = 0.0f;

    params.normal = normalize(v_Normal);

    params.view = normalize(u_SceneData.cameraPos - v_Position);
    params.nDotV = max(dot(params.normal, params.view), 0.0);

    vec3 light_contribution = params.albedo;

    vec3 ambient = params.albedo * 0.2f;

    o_Colour = vec4(light_contribution, 1.0f);
}