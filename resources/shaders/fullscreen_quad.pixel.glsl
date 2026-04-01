#version 460

layout(location = 0) out vec4 o_Colour;

layout (location = 0) in vec2 v_TexCoords;

//uniform float u_Time;
//uniform vec2 u_Res;

uniform sampler2D u_Tex;

void main()
{
    vec4 diff = texture(u_Tex, v_TexCoords);
    o_Colour = diff;
}