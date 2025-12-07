#version 450 core
#pragma stage : comp

//#include <Samplers.glslh>
layout (set = 3, binding = 0) uniform sampler r_DefaultSampler;
layout (set = 3, binding = 1) uniform sampler r_PointSampler;
layout (set = 3, binding = 2) uniform sampler r_LinearSampler;

vec4 SampleDefault(texture2D tex, vec2 texCoord)
{
    return texture(sampler2D(tex, r_DefaultSampler), texCoord);
}

vec4 SampleLinear(texture2D tex, vec2 texCoord)
{
    return texture(sampler2D(tex, r_LinearSampler), texCoord);
}

vec4 SampleLinear(texture2DArray tex, vec3 texCoord)
{
    return texture(sampler2DArray(tex, r_LinearSampler), texCoord);
}

vec4 SampleLinear(textureCube tex, vec3 texCoord)
{
    return texture(samplerCube(tex, r_LinearSampler), texCoord);
}

uvec4 SampleLinearLOD(utexture2D tex, vec2 texCoord, float lod)
{
    return textureLod(usampler2D(tex, r_LinearSampler), texCoord, lod);
}

vec4 SampleLinearLOD(texture2D tex, vec2 texCoord, float lod)
{
    return textureLod(sampler2D(tex, r_LinearSampler), texCoord, lod);
}

vec4 SampleLinearLOD(texture2DArray tex, vec3 texCoord, float lod)
{
    return texture(sampler2DArray(tex, r_LinearSampler), texCoord, lod);
}

vec4 SampleLinearLOD(textureCube tex, vec3 texCoord, float lod)
{
    return texture(samplerCube(tex, r_LinearSampler), texCoord, lod);
}

ivec2 GetTextureSize(texture2D tex)
{
    return textureSize(sampler2D(tex, r_DefaultSampler), 0);
}

ivec2 GetTextureSize(texture2D tex, int lod)
{
    return textureSize(sampler2D(tex, r_DefaultSampler), lod);
}


layout(binding = 0) uniform texture2D u_InputTexture;

layout(binding = 1) uniform writeonly image2D o_OutputTexture;

layout(push_constant) uniform PushConstants
{
    vec2 TexelSize;
    int SourceMipLevel;
} u_PushConstants;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    ivec2 outputCoord = ivec2(gl_GlobalInvocationID.xy);
    
    ivec2 outputSize = imageSize(o_OutputTexture);
    if (outputCoord.x >= outputSize.x || outputCoord.y >= outputSize.y)
        return;
    
    vec2 uv = (vec2(outputCoord) + 0.5) * u_PushConstants.TexelSize;
    
    vec4 color = SampleLinearLOD(u_InputTexture, uv, float(u_PushConstants.SourceMipLevel));
    imageStore(o_OutputTexture, outputCoord, color);
}
