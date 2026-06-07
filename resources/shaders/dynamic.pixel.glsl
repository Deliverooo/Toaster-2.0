#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(buffer_reference, std140) readonly buffer UBO 
{ 
    vec4 data;
};

layout(buffer_reference, std140) readonly buffer UBO2
{ 
    vec4 data;
};

layout(push_constant) uniform Push
{
    UBO u_UBO;
    UBO2 u_UBO2;
} pcs;


void main()
{

    vec3 colour = vec3(v_TexCoord, 0.0f);

    o_Colour = vec4(pcs.u_UBO.data);
}