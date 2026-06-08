#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout(push_constant) uniform PushConstants
{
    uint textureIndex;
    uint samplerIndex;

    Camera camera;
} pcs;

void main()
{
    gl_Position  = pcs.camera.proj * pcs.camera.view * vec4(a_Position.xy, 0.0f, 1.0f);
    //    gl_Position = vec4(a_Position.xy, 0.0f, 1.0f);

    v_TexCoord = a_TexCoord;
}