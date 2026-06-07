#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : require

layout (location = 0) in vec4   a_Position;
layout (location = 1) in vec4   a_Colour;
layout (location = 2) in vec2   a_TexCoords;
layout (location = 3) in float  a_TexIndex;
layout (location = 4) in float  a_TilingFactor;

layout (location = 0) out vec4   v_Colour;
layout (location = 1) out vec2   v_TexCoords;
layout (location = 2) out float  v_TexIndex;

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

invariant gl_Position;
void main()
{
    gl_Position = pcs.currentCameraPtr.u_Proj * pcs.currentCameraPtr.u_View * a_Position;

    v_Colour = a_Colour;
    v_TexCoords = a_TexCoords;
    v_TexIndex = a_TexIndex;
}