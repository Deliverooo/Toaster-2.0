#version 460

layout (location = 0) in vec4  a_Position;
layout (location = 1) in vec4  a_Colour;
layout (location = 2) in vec2  a_TexCoords;
layout (location = 3) in float  a_TexIndex;
layout (location = 4) in float  a_TilingFactor;

layout (location = 0) out vec2  v_TexCoords;
layout (location = 1) out vec4  v_Colour;

layout (std140, set = 1, binding = 0) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
};
layout (push_constant) uniform Transform
{
    mat4 model;
} _Transform_;

void main()
{
    v_Colour = a_Colour;
    v_TexCoords = a_TexCoords;
    gl_Position = u_Proj * u_View * _Transform_.model * a_Position;
}