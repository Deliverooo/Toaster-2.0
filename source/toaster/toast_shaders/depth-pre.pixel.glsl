#version 460

layout(location = 0) in vec3 v_Normal;

layout(location = 0) out vec3 o_Normal;

void main()
{
    o_Normal = normalize(v_Normal);
}
