#version 460

layout (location = 0) in vec2 v_TexCoords;

layout(location = 0) out vec4 o_Colour;

uniform vec4 u_Colour;

void main()
{
    o_Colour = u_Colour;
}