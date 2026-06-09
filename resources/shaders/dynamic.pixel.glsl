#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

#define IMAGE_STRIDE 32
#define BUFFER_STRIDE 16

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(descriptor_heap) uniform UBO
{
    vec4 colourData;
} ubos[];

layout(buffer_reference, std140) readonly buffer Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout(push_constant) uniform PushConstants
{
//    uint imageArrayOffset;
//    uint bufferArrayOffset;

    uint textureIndex;
    uint samplerIndex;

    Camera camera;
} pcs;

void main()
{
//    uint tex_index = (pc.imageArrayOffset / IMAGE_STRIDE) + pcs.textureIndex;
    vec4 texture_colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), v_TexCoord);

    o_Colour = texture_colour;
//    o_Colour = vec4(v_TexCoord, 0.0f, 1.0f);
}