#version 460

layout(location = 0) in vec3 v_Colour;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(binding = 1) uniform sampler2D u_Texture;

void main()
{
    vec4 diff = texture(u_Texture, v_TexCoord);

    o_Colour = diff * vec4(v_Colour, 1.0f);
}