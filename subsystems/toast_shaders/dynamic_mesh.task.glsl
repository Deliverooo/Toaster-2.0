#version 460
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define TASK_SHADER_DISPATCH_SIZE 32

layout(local_size_x = TASK_SHADER_DISPATCH_SIZE, local_size_y = 1, local_size_z = 1) in;

struct TaskPayload
{
    uint meshletIndices[TASK_SHADER_DISPATCH_SIZE];
};

taskPayloadSharedEXT TaskPayload payload;

shared uint visibleMeshletCount;

void main()
{
    uint threadId = gl_LocalInvocationID.x;
    uint meshletId = (gl_WorkGroupID.x * gl_WorkGroupSize.x) + threadId;

    if(threadId == 0)
    {
        visibleMeshletCount = 0;
    }
    barrier();


    if(true)
    {
        uint index = atomicAdd(visibleMeshletCount, 1);
        payload.meshletIndices[index] = meshletId;
    }
    barrier();

    if(threadId == 0)
    {
        EmitMeshTasksEXT(visibleMeshletCount, 1, 1);
    }

}
