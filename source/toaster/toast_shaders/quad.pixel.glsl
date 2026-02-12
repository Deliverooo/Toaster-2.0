#version 460

layout (location = 0) in vec2 v_TexCoords;

layout(location = 0) out vec4 o_Colour;

uniform sampler2D u_Texture;

void main()
{
    vec4 diff = texture(u_Texture, v_TexCoords);

    o_Colour = diff;
}