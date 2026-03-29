#version 460

layout (location = 0) in vec4  a_Position;
layout (location = 1) in vec4  a_Colour;
layout (location = 2) in vec2  a_TexCoords;
layout (location = 3) in float a_TexIndex;
layout (location = 4) in float a_TilingFactor;
layout (location = 5) in uint  a_ObjectID;

layout (location = 0) out vec2  v_TexCoords;
layout (location = 1) out vec4  v_Colour;
layout (location = 2) out float v_TexIndex;
layout (location = 3) out float v_TilingFactor;

uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    v_Colour = a_Colour;
    v_TexCoords = a_TexCoords;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
    gl_Position = u_Proj * u_View * a_Position;
}