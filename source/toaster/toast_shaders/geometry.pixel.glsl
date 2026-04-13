#version 460

layout(location = 0) in vec3 v_Colour;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_Texture;

layout(push_constant) uniform Material
{
    vec3 albedoColour;
} u_Material;

void main()
{
    vec3 col = texture(u_Texture, v_TexCoord).rgb;

    col *= u_Material.albedoColour;

    o_Colour = vec4(col, 1.0f);
}