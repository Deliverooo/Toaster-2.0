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


// Reconstruct View-Space position from screen-space coordinates and depth
vec3 getViewPos(vec2 uv) {
    float depth = texture(u_DepthTex, uv).r;
    // Convert to Normalized Device Coordinates (NDC)
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = u_InvProj * ndc;
    return viewPos.xyz / viewPos.w;
}

// AO Parameters
const int SAMPLES = 8;
const float RADIUS = 0.5; // Sampling radius
const float BIAS = 0.02;  // Helps reduce self-shadowing artifacts

// Fast pseudo-random generator
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 texSize = textureSize(u_DepthTex, 0);

    // Bounds check
    if (pixelCoords.x >= texSize.x || pixelCoords.y >= texSize.y) return;

    // Map pixel to [0.0, 1.0] UV coordinates
    vec2 uv = (vec2(pixelCoords) + vec2(0.5)) / vec2(texSize);

    // Get view space position of target pixel
    vec3 originPos = getViewPos(uv);

    // Quick random angle rotation based on pixel coordinates
    float angle = rand(uv) * 6.2831853;
    vec2 rotation = vec2(cos(angle), sin(angle));

    float occlusion = 0.0;

    // 2D disk sample direction offsets
    vec2 sampleOffsets[8] = vec2[](
    vec2(1.0, 0.0), vec2(-1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, -1.0),
    vec2(0.7, 0.7), vec2(-0.7, -0.7), vec2(0.7, -0.7), vec2(-0.7, 0.7)
    );

    for (int i = 0; i < SAMPLES; ++i) {
        // Rotate the offset direction to hide banding artifacts
        vec2 rotatedOffset = vec2(
        sampleOffsets[i].x * rotation.x - sampleOffsets[i].y * rotation.y,
        sampleOffsets[i].x * rotation.y + sampleOffsets[i].y * rotation.x
        );

        // Scale offset based on sample depth to adjust real-world radius size
        vec2 sampleUV = uv + (rotatedOffset * RADIUS) / -originPos.z;

        // Retrieve neighboring geometry position
        vec3 samplePos = getViewPos(sampleUV);

        // Vector pointing from origin pixel to sample point geometry
        vec3 v = samplePos - originPos;

        // Compute occlusion factor using a classic range check
        // If sample point is significantly far behind, ignore it to prevent haloing
        float dist = length(v);
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / dist);

        // Accumulate occlusion if the sample point is in front of origin depth
        occlusion += (v.z >= BIAS ? 1.0 : 0.0) * rangeCheck;
    }

    // Normalize and invert the result (1.0 = completely unoccluded, 0.0 = fully occluded)
    float aoFactor = 1.0 - (occlusion / float(SAMPLES));

    // Write out the result
    imageStore(o_Occlusion, pixelCoords, vec4(aoFactor));
}