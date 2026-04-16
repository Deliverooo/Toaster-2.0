#version 460

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
};
layout(set = 1, binding = 2) uniform PointLightData
{
    uint count;
    PointLight lights[64];
} u_PointLights;


void main()
{
    vec3 norm = normalize(v_Normal);

    vec3 result = vec3(0.0f);

    vec3 ambient = texture(u_AlbedoTexture, v_TexCoord).rgb * 0.2f;

    for (uint i = 0; i < u_PointLights.count; i++)
    {
        vec3 lightDir = normalize(u_PointLights.lights[i].position - v_Position);
        float diff = max(dot(norm, lightDir), 0.0);

        vec3 col = texture(u_AlbedoTexture, v_TexCoord).rgb;
        col *= u_Material.albedoColour;
        col *= diff;

        result += (col + ambient);
    }

    o_Colour = vec4(result, 1.0f);
}