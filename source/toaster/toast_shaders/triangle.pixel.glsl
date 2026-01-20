#version 460

layout(location = 0) in vec3 v_Colour;
layout(location = 0) out vec4 o_Colour;

uniform float u_Tst;

void main()
{
    o_Colour = vec4(u_Tst * v_Colour, 1.0f);
}