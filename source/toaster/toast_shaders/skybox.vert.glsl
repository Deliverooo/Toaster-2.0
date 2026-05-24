#version 460

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Position;

layout(std140, set = 1, binding = 1) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

void main()
{
    gl_Position = vec4(a_Position.xy, 0.0f, 1.0f);

    vec4 viewPos = inverse(u_Proj) * vec4(a_Position.xy, 0.0f, 1.0f);
    v_Position = mat3(inverse(u_View)) * viewPos.xyz;
}