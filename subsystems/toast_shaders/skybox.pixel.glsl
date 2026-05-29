#version 460

layout(location = 0) in vec3 v_Position;

layout(location = 0) out vec4 o_Colour;

layout(set = 1, binding = 2) uniform samplerCube u_CubemapImage;

void main()
{
    o_Colour = textureLod(u_CubemapImage, normalize(v_Position), 1) * 0.5f;
}