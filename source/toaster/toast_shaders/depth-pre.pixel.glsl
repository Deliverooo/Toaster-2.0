#version 460

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_Position;

layout(location = 0) out vec3 o_Normal;
layout(location = 1) out vec3 o_Position;

void main()
{
    o_Normal = normalize(v_Normal);
    o_Position = v_Position;
}
