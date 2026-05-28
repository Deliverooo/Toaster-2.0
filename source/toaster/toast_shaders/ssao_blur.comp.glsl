#version 460

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 1, binding = 0) uniform sampler2D u_Occlusion;

layout(set = 1, binding = 1) uniform writeonly image2D o_BlurredOcclusion;

void main()
{
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 image_size = imageSize(o_BlurredOcclusion);

    if (pixel_coords.x >= image_size.x || pixel_coords.y >= image_size.y) return;

    vec2 uv = (vec3(pixel_coords, 0.0f).xy + 0.5f) / vec2(image_size);

    vec2 texel_size = 1.0f / vec2(textureSize(u_Occlusion, 0));
    float result = 0.0f;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            result += texture(u_Occlusion, uv + offset).r;
        }
    }

    imageStore(o_BlurredOcclusion, pixel_coords, vec4(result / (4.0f * 4.0f)));
}
