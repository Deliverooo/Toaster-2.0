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

vec3 light_pos = vec3(0.0f, -1.0f, 0.0f);
void main()
{
    vec3 col = texture(u_AlbedoTexture, v_TexCoord).rgb;
    col *= u_Material.albedoColour;

    vec3 ambient = vec3(0.05f);

    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(light_pos - v_Position);
    float diff = max(dot(norm, lightDir), 0.0);

    col *= (diff + ambient);

    o_Colour = vec4(col, 1.0f);
}