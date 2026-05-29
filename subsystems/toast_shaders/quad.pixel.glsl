#version 460

layout(location = 0) out vec4 o_Colour;

layout (location = 0) in vec4        v_Colour;
layout (location = 1) in vec2        v_TexCoords;
layout (location = 2) in flat float  v_TexIndex;

layout(set = 0, binding = 0) uniform sampler2D u_Textures[32];

void main()
{
    vec4 diff = texture(u_Textures[int(v_TexIndex)], v_TexCoords);
    o_Colour = diff * v_Colour;
}