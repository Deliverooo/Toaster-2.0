#version 460

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(set = 1, binding = 1) uniform writeonly image2D u_TestImage;

void main()
{
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec4 colour = vec4(1.0f, 0.5f, 0.0f, 1.0f);

    imageStore(u_TestImage, texelCoord, colour);
}
