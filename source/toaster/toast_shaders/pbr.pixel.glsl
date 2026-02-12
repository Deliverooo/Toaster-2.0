#version 460

layout (location = 0) in vec2 v_WorldPos;
layout (location = 1) in vec2 v_Normal;
layout (location = 2) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

void main()
{
    o_Colour = vec4(1.0f);
}


