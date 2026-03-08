#version 460

layout (location = 0) in vec4  a_Position;
layout (location = 1) in vec4  a_Colour;
layout (location = 2) in vec2  a_TexCoords;
layout (location = 3) in float a_TexIndex;

layout (location = 0) out vec2  v_TexCoords;
layout (location = 1) out vec4  v_Colour;
layout (location = 2) out float v_TexIndex;

uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    v_Colour = a_Colour;
    v_TexCoords = a_TexCoords;
    v_TexIndex = a_TexIndex;
    gl_Position = u_Proj * u_View * a_Position;
}