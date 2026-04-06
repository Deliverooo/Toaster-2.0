#version 460

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 0) uniform sampler2D u_Texture;

void main()
{

    vec4 composite = texture(u_Texture, v_TexCoord);

    o_Colour = vec4(composite.rgb, 1.0f);
}