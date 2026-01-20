#version 460

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Colour;

layout (location = 0) out vec3 o_Colour;

void main()
{
    gl_Position = vec4(a_Position.x, a_Position.y, a_Position.z, 1.0);
    o_Colour = a_Colour;
}