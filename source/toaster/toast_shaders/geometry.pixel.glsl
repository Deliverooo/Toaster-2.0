#version 460

layout(location = 0) in vec3 v_Colour;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_Texture;
layout(set = 1, binding = 0) uniform sampler2D u_2D;

void main()
{

    vec3 col = texture(u_Texture, v_TexCoord).rgb;
    col *= texture(u_2D, v_TexCoord).rgb;

    o_Colour = vec4(col, 1.0f);
}