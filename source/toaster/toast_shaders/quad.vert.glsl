#version 460

layout (location = 0) in vec4   a_Position;
layout (location = 1) in vec4   a_Colour;
layout (location = 2) in vec2   a_TexCoords;
layout (location = 3) in float  a_TexIndex;
layout (location = 4) in float  a_TilingFactor;

layout (location = 0) out vec4   v_Colour;
layout (location = 1) out vec2   v_TexCoords;
layout (location = 2) out float  v_TexIndex;

layout(std140, set = 1, binding = 1) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

layout (push_constant) uniform Transform
{
    mat4 model;
} _Transform_;

invariant gl_Position;
void main()
{
    gl_Position = u_Proj * u_View * _Transform_.model * a_Position;

    v_Colour = a_Colour;
    v_TexCoords = a_TexCoords;
    v_TexIndex = a_TexIndex;
}