#version 460

layout(location = 0) out vec4 o_Colour;

layout (location = 0) in vec2       v_TexCoords;
layout (location = 1) in vec4       v_Colour;


void main()
{
    o_Colour = v_Colour;
}