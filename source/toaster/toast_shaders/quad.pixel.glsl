#version 460

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int  o_ObjectID;

layout (location = 0) in vec2       v_TexCoords;
layout (location = 1) in vec4       v_Colour;
layout (location = 2) flat in float v_TexIndex;
layout (location = 3) in float      v_TilingFactor;
layout (location = 4) flat in int   v_ObjectID;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
    vec4 tex_colour = texture(u_Textures[int(v_TexIndex)], v_TexCoords * v_TilingFactor) * v_Colour;

    o_Colour   = tex_colour;
    o_ObjectID = v_ObjectID;
}