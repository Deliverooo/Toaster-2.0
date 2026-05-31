#version 460

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 2, binding = 0) uniform sampler2D u_PositionsTex;
layout(set = 2, binding = 1) uniform sampler2D u_NormalsTex;
layout(set = 2, binding = 2) uniform sampler2D u_NoiseTex;
layout(set = 2, binding = 3) uniform writeonly image2D o_Occlusion;

layout(std140, set = 1, binding = 1, row_major) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

layout(std140, set = 3, binding = 0) uniform SSAOKernel
{
    vec4 u_Samples[64];
};

layout(push_constant, row_major) uniform Constants
{
    float u_Radius;
    float u_Bias;
    vec2 u_NoiseScale;
};


float linearizeDepth(float p_depth_sample)
{
    float near = 0.1f;
    float far = 1000.0f;
    return (2.0 * near) / (far + near - p_depth_sample * (far - near));
}

vec3 getPositionAtUV(vec2 p_uv)
{
    return texture(u_PositionsTex, p_uv).xyz;
//    float depth = texture(u_DepthTex, p_uv).r;
//    vec4 ndc = vec4(p_uv.x * 2.0f - 1.0f, (1.0f - p_uv.y) * 2.0f - 1.0f, depth, 1.0f);
//
//    vec4 view_pos_clip = u_InvProj * ndc;
//    vec3 view_pos = view_pos_clip.xyz / view_pos_clip.w;
//    return view_pos;
}

void main()
{
    float radius = u_Radius;
    //    float radius = 0.5f;
    float bias = u_Bias;
    //    float bias = 0.025f;
    vec2 noise_scale = u_NoiseScale;
    //    vec2 noise_scale = vec2(1/240.0f, 1/135.0f);

    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 image_size = imageSize(o_Occlusion);

    if (pixel_coords.x >= image_size.x || pixel_coords.y >= image_size.y) return;

    vec2 uv = (vec3(pixel_coords, 0.0f).xy + 0.5f) / vec2(image_size);

    vec3 frag_pos = getPositionAtUV(uv);
    vec3 normal = texture(u_NormalsTex, uv).xyz;

    vec3 random_vec = texture(u_NoiseTex, uv * noise_scale).xyz;

    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    int kernel_size = 64;

    for (int i = 0; i < kernel_size; i++)
    {
        vec3 sample_pos = TBN * u_Samples[i].xyz;
        sample_pos = frag_pos + sample_pos * radius;

        vec4 offset = vec4(sample_pos, 1.0f);
        offset  = u_Proj * offset;
        offset.xyz /= abs(offset.w);

        offset.x = offset.x * 0.5f + 0.5f;
        offset.y = 1.0f - (offset.y * 0.5f + 0.5f);

        if (offset.x < 0.0f || offset.x > 1.0f || offset.y < 0.0f || offset.y > 1.0f)
        {
            continue;
        }

        float sample_depth = getPositionAtUV(offset.xy).z;

        float range_check = smoothstep(0.0f, 1.0f, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0f : 0.0f) * range_check;
    }

    float final_occlusion = 1.0f - (occlusion / float(kernel_size));

    imageStore(o_Occlusion, pixel_coords, vec4(vec3(final_occlusion), 1.0f));
}