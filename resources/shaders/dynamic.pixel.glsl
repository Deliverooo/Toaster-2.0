#version 460

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

void main()
{

    vec3 colour = vec3(v_TexCoord, 0.0f); 
    o_Colour = vec4(colour, 1.0f);
}