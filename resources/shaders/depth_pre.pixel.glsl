#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;

layout(location = 0) out vec3 o_Normal;
layout(location = 1) out vec3 o_Position;

// layout(descriptor_heap) uniform texture2D globalTextures[];
// layout(descriptor_heap) uniform sampler globalSamplers[];


layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout(push_constant) uniform PushConstants
{
    Camera camera;
    uint _cameraPadding[2];

    mat4 modelMatrix;

} pcs;

void main()
{
    o_Normal = normalize(v_Normal);
    o_Position = v_Position;
}