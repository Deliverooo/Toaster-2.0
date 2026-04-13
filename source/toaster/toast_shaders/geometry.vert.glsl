#version 460

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Colour;
layout(location = 1) out vec2 v_TexCoord;

layout(std140, set = 1, binding = 1) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
};

layout (push_constant) uniform Transform
{
    mat4 u_Model;
};

void main()
{
    gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0f);

    v_Colour = a_Normal;
    v_TexCoord = a_TexCoord;
}