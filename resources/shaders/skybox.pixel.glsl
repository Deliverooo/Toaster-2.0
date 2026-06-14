#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform textureCube globalCubeMaps[];

layout(descriptor_heap) uniform sampler globalSamplers[];

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout(push_constant) uniform PushConstants
{
    Camera camera;

    uint samplerId;
    uint skyboxMapId;

} pcs;

void main()
{
    vec4 texture_colour = textureLod(samplerCube(globalCubeMaps[pcs.skyboxMapId], globalSamplers[pcs.samplerId]), normalize(v_Position), 1);
    // vec4 texture_colour =imageLoad(globalCubeMaps, );

    o_Colour = vec4(texture_colour.rgb, 1.0f);
}