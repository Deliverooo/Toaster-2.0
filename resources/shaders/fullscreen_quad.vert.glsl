#version 460

layout (location = 0) in vec3  a_Position;
layout (location = 1) in vec2  a_TexCoords;

layout (location = 0) out vec2  v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    v_TexCoords = a_TexCoords;
    gl_Position = u_Proj * u_View * vec4(a_Position, 1.0f);
}