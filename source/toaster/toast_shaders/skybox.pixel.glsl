#version 460

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_Texture;
layout(set = 1, binding = 2) uniform  samplerCube  u_CubemapImage;


void main()
{
    vec3 view_dir = normalize(v_Position);


    o_Colour = texture(u_CubemapImage, view_dir);
}