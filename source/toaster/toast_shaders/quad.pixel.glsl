#version 460

layout(location = 0) out vec4 o_Colour;

layout (location = 0) in vec2       v_TexCoords;
layout (location = 1) in vec4       v_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_WhiteTexture;

void main()
{
    vec4 diff = texture(u_WhiteTexture, v_TexCoords);
    o_Colour = diff * v_Colour;
}