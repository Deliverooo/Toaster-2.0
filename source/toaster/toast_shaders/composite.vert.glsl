#version 460

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_Model;
    mat4 u_View;
    mat4 u_Proj;
};

void main()
{
    gl_Position = u_Proj * u_View * vec4(a_Position, 1.0f);
    v_TexCoord = a_TexCoord;
}