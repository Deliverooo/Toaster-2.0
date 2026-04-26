#version 460

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, set = 1, binding = 0) buffer Test
{
    int data[];
} s_OutputBuffer;

void main()
{
    uint index = gl_GlobalInvocationID.x;
    s_OutputBuffer.data[index] = 6741;
}
