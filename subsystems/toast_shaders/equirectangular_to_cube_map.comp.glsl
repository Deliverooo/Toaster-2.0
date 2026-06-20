#version 460
#extension GL_GOOGLE_include_directive : require
#include "common.glslh"

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(descriptor_heap, rgba16f) restrict uniform writeonly imageCube o_Cubemap[];

layout(push_constant) uniform PushConstants
{
    uint equirectangularMapId;
    uint cubeMapId;
    uint samplerId;
} pcs;

vec3 getCubeMapTexCoord(vec2 imageSize)
{
    vec2 st = gl_GlobalInvocationID.xy / imageSize;
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

void main()
{
    vec3 cube_uv = getCubeMapTexCoord(vec2(imageSize(o_Cubemap[pcs.cubeMapId])));

    float phi = atan(cube_uv.z, cube_uv.x);
    float theta = acos(cube_uv.y);
    vec2 uv = vec2(phi / (2.0f * PI) + 0.5f, theta / PI);

    vec4 colour = texture(sampler2D(texture2DHeap[pcs.equirectangularMapId], samplerHeap[pcs.samplerId]), uv);
    colour = min(colour, vec4(500.0f));

    imageStore(o_Cubemap[pcs.cubeMapId], ivec3(gl_GlobalInvocationID), colour);
}