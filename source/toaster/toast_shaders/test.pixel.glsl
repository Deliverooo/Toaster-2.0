#version 460

layout(location = 0) in vec3 fragColour;

layout(location = 0) out vec4 o_Colour;


void main()
{
    vec3 diff = fragColour;
    o_Colour = vec4(diff, 1.0f);
}