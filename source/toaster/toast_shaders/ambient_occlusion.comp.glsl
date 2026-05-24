#version 460

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 2, binding = 0) uniform sampler2D u_DepthTex;
layout(set = 2, binding = 1) uniform sampler2D u_NormalsTex;
layout(set = 2, binding = 2) uniform writeonly image2D o_Occlusion;

layout(std140, set = 1, binding = 1) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

layout(push_constant) uniform Constants
{
    layout(offset = 0) float u_Radius;
    layout(offset = 4) float u_Thickness;

    layout(offset = 8) uint u_FrameIndex;

    layout(offset = 12) int u_NumSlices;
    layout(offset = 16) int u_NumSamplesPerSlice;
};


// Converts screen space depth to view space coordinates
vec3 GetViewPos(vec2 uv) {
    float depth = texture(u_DepthTex, uv).r;
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = u_InvProj * clipPos;
    return viewPos.xyz / viewPos.w;
}

float Hash21(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void main()
{
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = (vec2(texelCoord) + 0.5) / vec2(textureSize(u_DepthTex, 0));

    vec3 viewPos = GetViewPos(uv);
    vec3 viewNormal = normalize(texture(u_NormalsTex, uv).xyz);

    // Initial visibility bitmask: 0 = completely visible, 1 = occluded
    // Using a 32-bit integer representing 32 angular sectors of a 180-degree hemisphere slice
    uint visibilityBitmask = 0u;

    // Interleaved gradient noise or blue noise for sampling angle rotation
    float noise = Hash21(vec2(texelCoord) + vec2(float(u_FrameIndex) * 17.0));
    float randomAngle = noise * 6.28318530718;

    for (int s = 0; s < u_NumSlices; ++s) {
        float sliceAngle = (float(s) / float(u_NumSlices)) * 3.14159265 + randomAngle;
        vec2 sliceDir = vec2(cos(sliceAngle), sin(sliceAngle));

        for (int i = 1; i <= u_NumSamplesPerSlice; ++i) {
            // Step out along the slice direction in screen space
            float stepDist = (float(i) / float(u_NumSamplesPerSlice)) * u_Radius;
            vec2 sampleUV = uv + (sliceDir * stepDist / viewPos.z);// Scale by depth

            vec3 sampleViewPos = GetViewPos(sampleUV);
            vec3 horizonVec = sampleViewPos - viewPos;

            // Calculate the angle of this horizon vector relative to the slice tangent
            float distanceSq = dot(horizonVec, horizonVec);

            if (distanceSq < u_Radius * u_Radius) {
                // Project the sample onto our 2D slice plane to evaluate the angular sector
                float angle = atan(horizonVec.z, length(horizonVec.xy));

                // Map angle [-PI/2, PI/2] to bitmask sector range [0, 31]
                float normalizedAngle = (angle + 1.570796) / 3.14159265;
                int frontSector = clamp(int(normalizedAngle * 32.0), 0, 31);

                // Account for object thickness to build the back-face sector boundary
                float backAngle = atan((horizonVec.z - u_Thickness), length(horizonVec.xy));
                float normalizedBackAngle = (backAngle + 1.570796) / 3.14159265;
                int backSector = clamp(int(normalizedBackAngle * 32.0), 0, 31);

                // Create a bitmask for the range [backSector, frontSector]
                uint sampleMask = 0u;
                for (int bit = backSector; bit <= frontSector; ++bit) {
                    sampleMask |= (1u << bit);
                }

                // Accumulate the occluded sectors into our master bitmask
                visibilityBitmask |= sampleMask;
            }
        }
    }

    // Evaluate final ambient occlusion by counting the remaining unoccluded bits (zeros)
    // bitCount returns the number of set bits (ones), which represent occlusion
    int occludedSectors = bitCount(visibilityBitmask);
    float ambientOcclusion = 1.0 - (float(occludedSectors) / 32.0);

    // Apply lighting/normal weight alignment adjustments here if desired

    //    imageStore(o_Occlusion, texelCoord, vec4(0.0f, 1.0f, 0.0f, 1.0f));

    imageStore(o_Occlusion, texelCoord, vec4(vec3(ambientOcclusion), 1.0f));
}