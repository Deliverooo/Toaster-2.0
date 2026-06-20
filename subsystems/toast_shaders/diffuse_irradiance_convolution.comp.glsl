#version 460
#extension GL_GOOGLE_include_directive : require
#include "common.glslh"

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(descriptor_heap, rgba16f) restrict uniform writeonly imageCube o_Cubemap[];

layout(push_constant) uniform PushConstants
{
    uint environmentMapId;
    uint diffuseIrradianceMapId;
    uint samplerId;
} pcs;

vec3 getCubeMapDirection(ivec3 p_id, vec2 p_size)
{
    vec2 uv = (vec2(p_id.xy) + 0.5f) / p_size;
    vec2 ndc = uv * 2.0f - 1.0f;

    vec3 dir = vec3(0.0f);

    switch (p_id.z)
    {
        case 0: dir = vec3(1.0f, -ndc.y, -ndc.x); break;
        case 1: dir = vec3(-1.0f, -ndc.y, ndc.x); break;
        case 2: dir = vec3(ndc.x, 1.0f, ndc.y); break;
        case 3: dir = vec3(ndc.x, -1.0f, -ndc.y); break;
        case 4: dir = vec3(ndc.x, -ndc.y, 1.0f); break;
        case 5: dir = vec3(-ndc.x, -ndc.y, -1.0f); break;
    }
    return normalize(dir);
}


void main()
{
    ivec3 texel_coord = ivec3(gl_GlobalInvocationID.xyz);
    ivec2 image_size = imageSize(o_Cubemap[pcs.diffuseIrradianceMapId]);
    if (texel_coord.x >= image_size.x || texel_coord.y >= image_size.y) return;

    vec3 normal = getCubeMapDirection(texel_coord, image_size);

    vec3 irradiance = vec3(0.0f);

    vec3 up = abs(normal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sample_delta = 0.025f;
    float num_samples = 0.0f;

    for (float phi = 0.0f; phi < 2.0f * PI; phi += sample_delta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sample_delta)
        {
            vec3 tangent_sample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

            irradiance += texture(samplerCube(textureCubeHeap[pcs.environmentMapId], samplerHeap[pcs.samplerId]), sample_vec).rgb * cos(theta) * sin(theta);
            num_samples++;
        }
    }

    irradiance = PI * irradiance * (1.0f / num_samples);

    imageStore(o_Cubemap[pcs.diffuseIrradianceMapId], texel_coord, vec4(irradiance, 1.0f));
}
