#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(location = 0) in vec2 m_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(push_constant) uniform Constants
{
    uint64_t vertexBuffer;
    uint64_t meshletBuffer;
    uint64_t meshletVertexIndexBuffer;
    uint64_t meshletTriangleIndexBuffer;

    uint64_t camera;

    uint samplerIndex;
    uint textureIndex;
} pcs;

vec3 getSubmeshDebugColour(uint p_submesh_index)
{
    switch(p_submesh_index)
    {
        case 0: return vec3(0.0f);
        case 1: return vec3(1.0f, 0.0f, 0.0f);
        case 2: return vec3(0.0f, 1.0f, 0.0f);
        case 3: return vec3(0.0f, 1.0f, 1.0f);
        case 4: return vec3(1.0f, 1.0f, 1.0f);
        case 5: return vec3(1.0f ,1.0f, 0.0f);
        case 6: return vec3(0.5f, 0.25f, 0.8f);
        case 7: return vec3(0.76f, 1.0f, 0.1f);
        case 8: return vec3(1.0f, 0.1f, 0.1f);
    }

    return vec3(1.0f, 0.0f, 1.0f);
}

void main()
{
    vec4 texture_colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), m_TexCoord);

    //    o_Colour = texture_colour;

    o_Colour = vec4(getSubmeshDebugColour(m_SubmeshIndex), 1.0f);
}