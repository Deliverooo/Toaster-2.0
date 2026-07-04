#version 460
#extension GL_GOOGLE_include_directive: require
#include "common.glslh"

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout (descriptor_heap, rgba16f) restrict uniform writeonly imageCube o_Cubemap[];

layout (push_constant) uniform PushConstants
{
    uint environmentMapId;
    uint diffuseIrradianceMapId;
    uint samplerId;

    uint numSamples;
    float roughness;
} pcs;


// Van der Corput radical inverse bit-twiddling
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// Importance sampling using GGX
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // tangent-to-world space
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// Geometry shadowing function (Smith)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float a = roughness;
    float k = (a * a) / 2.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

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
    vec3 view = normal;

    vec3 prefiliter_colour = vec3(0.0f);
    float total_weight = 0.0f;

    uint numSamples = pcs.numSamples;
    float roughness = pcs.roughness;

    for (uint i = 0u; i < numSamples; ++i)
    {
        vec2 Xi = Hammersley(i, numSamples);
        vec3 H = ImportanceSampleGGX(Xi, normal, roughness);
        vec3 L = normalize(2.0 * dot(view, H) * H - view);

        float NdotL = max(dot(normal, L), 0.0);
        if (NdotL > 0.0)
        {
            // Sample from the environment map
            prefiliter_colour += texture(samplerCube(textureCubeHeap[pcs.environmentMapId], samplerHeap[pcs.samplerId]), L).rgb * NdotL;
            total_weight += NdotL;
        }
    }

    prefiliter_colour = prefiliter_colour / max(total_weight, 0.001);

    imageStore(o_Cubemap[pcs.diffuseIrradianceMapId], texel_coord, vec4(prefiliter_colour, 1.0f));
}
