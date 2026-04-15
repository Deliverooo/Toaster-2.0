#version 460

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_Texture;

void main()
{
    o_Colour = texture(u_Texture, v_TexCoord);
}