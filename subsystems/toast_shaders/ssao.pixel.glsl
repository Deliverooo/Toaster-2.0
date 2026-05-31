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

vec3 getViewPos(vec2 p_uv)
{
    return texture(u_PositionsTex, p_uv).xyz;
}

const float PI = 3.14159265359;

void main()
{
//    int num_directions = 2;
//    int num_steps = 4;
//
//    vec3 viewPos = getViewPos(v_TexCoord);
//    vec3 viewNormal = texture(u_NormalsTex, v_TexCoord).xyz; // Must be in view-space
//    viewNormal = normalize(viewNormal);
//
//    // Get noise for rotation and step offset
////    vec2 noiseUV = fragTexCoord * (vec2(textureSize(u_NormalsTex, 0)) / 4.0f);
//    vec3 noise = texture(u_NoiseTex,  v_TexCoord * u_NoiseScale).rgb;
//
//    float visibility = 0.0;
//
//    // Calculate pixel radius in texture space
//    vec4 projRadius = u_Proj * vec4(u_Radius, 0.0, viewPos.z, 1.0);
//    float texRadius = (projRadius.x / projRadius.w) * 0.5;
//
//    // Loop over search directions
//    for (int d = 0; d < num_directions; ++d) {
//        // Rotate the base direction using noise
//        float angle = (float(d) + noise.x) * (PI / float(num_directions));
//        vec2 dir = vec2(cos(angle), sin(angle));
//
//        // Start horizons at the tangent plane angle (initially flat)
//        float h1 = -1.0;
//        float h2 = -1.0;
//
//        // Ray-march along the direction vector
//        for (int s = 1; s <= num_steps; ++s) {
//            // Jitter step distance to reduce banding
//            float stepFraction = (float(s) + noise.y) / float(num_steps);
//            vec2 uvOffset = dir * texRadius * stepFraction;
//
//            // Sample horizon positive direction
//            vec3 posHorizon = getViewPos(v_TexCoord + uvOffset);
//            vec3 deltaPos = posHorizon - viewPos;
//            if (length(deltaPos) < u_Radius) {
//                float h = dot(viewNormal, normalize(deltaPos));
//                h1 = max(h1, h);
//            }
//
//            // Sample horizon negative direction
//            vec3 negHorizon = getViewPos(v_TexCoord - uvOffset);
//            vec3 deltaNeg = negHorizon - viewPos;
//            if (length(deltaNeg) < u_Radius) {
//                float h = dot(viewNormal, normalize(deltaNeg));
//                h2 = max(h2, h);
//            }
//        }
//
//        // Convert dot products back to angles and constrain
//        h1 = acos(clamp(h1, -1.0, 1.0));
//        h2 = acos(clamp(h2, -1.0, 1.0));
//
//        // Calculate analytical visibility for this slice
//        // GTAO projects the slice onto the normal to find unoccluded arc
//        visibility += (sin(h1) + sin(h2)) * 0.5;
//    }
//
//    // Average the results across all directions
//    visibility /= float(num_directions);
//
//    o_Occlusion = vec4(clamp(visibility, 0.0, 1.0)).rgb;

    vec3 frag_pos = texture(u_PositionsTex, v_TexCoord).rgb;
    vec3 normal = normalize(texture(u_NormalsTex, v_TexCoord).rgb);

    vec2 noise_scale    = u_NoiseScale;

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

    occlusion = clamp(occlusion, 0.2f, 1.0f);
    o_Occlusion = vec3(occlusion);
}
