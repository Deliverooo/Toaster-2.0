#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : require

layout(location = 0) out vec4 o_Colour;

layout (location = 0) in vec4        v_Colour;
layout (location = 1) in vec2        v_TexCoords;
layout (location = 2) in flat float  v_TexIndex;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

layout(push_constant) uniform PushConstants
{
    uint textureIndex;
    uint samplerIndex;

    Camera currentCameraPtr;
} pcs;

void main()
{
    vec4 diff = texture(sampler2D(globalTextures[int(v_TexIndex)], globalSamplers[pcs.samplerIndex]), v_TexCoords);
    o_Colour = diff * v_Colour;
}