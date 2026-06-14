#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_Normal;

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
    Camera camera;
    uint _cameraPadding[2];

    mat4 modelMatrix;

    vec4 albedoColour;

    uint samplerIndex;

    uint albedoMap;
    uint normalMap;
    uint hasNormalMap;

    float roughness;
    float metalness;

    float _padd[2];

} pcs;

const vec3 temp_light_pos = vec3(0.0f, 1.0f,0.0f);

void main()
{
    vec4 texture_colour = texture(sampler2D(globalTextures[pcs.albedoMap], globalSamplers[pcs.samplerIndex]), v_TexCoord);

    vec4 final_colour = pcs.albedoColour;

    vec3 light_dir = normalize(temp_light_pos - v_WorldPos);

    float diff = max(dot(light_dir, normalize(v_Normal)), 0.0f);

    final_colour.rgb *= texture_colour.rgb * diff;
    o_Colour = final_colour;
    // o_Colour = vec4(1.0f)
}