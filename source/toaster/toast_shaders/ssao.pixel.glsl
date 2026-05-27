#version 460

layout(set = 2, binding = 0) uniform sampler2D u_PositionsTex;
layout(set = 2, binding = 1) uniform sampler2D u_NormalsTex;
layout(set = 2, binding = 2) uniform sampler2D u_NoiseTex;

layout(std140, set = 1, binding = 1) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

layout(std140, set = 3, binding = 0) uniform SSAOKernel
{
    vec4 u_Samples[64];
};

layout(push_constant) uniform Constants
{
    float u_Radius;
    float u_Bias;
    vec2 u_NoiseScale;
};

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec3 o_Occlusion;

#define KERNEL_SIZE 64u

void main()
{
    vec3 frag_pos = texture(u_PositionsTex, v_TexCoord).rgb;
    vec3 normal = normalize(texture(u_NormalsTex, v_TexCoord).rgb);

    vec2 screen_size    = vec2(textureSize(u_PositionsTex, 0));
    vec2 noise_tex_size = vec2(textureSize(u_NoiseTex, 0));
    vec2 noise_scale    = screen_size / noise_tex_size;

    vec3 random_vec = texture(u_NoiseTex, v_TexCoord * noise_scale).xyz;

    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    for (uint i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 sample_pos = TBN * u_Samples[i].xyz;
        sample_pos = frag_pos + sample_pos * u_Radius;

        vec4 offset = vec4(sample_pos, 1.0f);
        offset = u_Proj * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f;

        float sample_depth = texture(u_PositionsTex, offset.xy).z;

        float range_check = smoothstep(0.0f, 1.0f, u_Radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + u_Bias ? 1.0f : 0.0f) * range_check;
    }
    occlusion = 1.0f - (occlusion / float(KERNEL_SIZE));

    o_Occlusion = vec3(occlusion);
}
