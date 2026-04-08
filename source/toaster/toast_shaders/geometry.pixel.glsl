#version 460

layout(location = 0) in vec3 v_Colour;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

//layout(set = 0, binding = 1) uniform sampler2D u_Texture;
//layout(set = 0, binding = 2) uniform sampler2D u_Texture2;

layout(push_constant) uniform Material
{
    float roughness;
} material;


void main()
{

//    vec3 col = texture(u_Texture, v_TexCoord).rgb;

    o_Colour = vec4(v_TexCoord, 0.0f, 1.0f);
}