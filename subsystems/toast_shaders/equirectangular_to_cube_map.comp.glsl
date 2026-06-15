#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_descriptor_heap : enable

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(descriptor_heap) uniform texture2D globalTextures[];
layout(descriptor_heap, rgba16f) restrict uniform writeonly imageCube globalCubeMaps[];
layout(descriptor_heap) uniform sampler globalSamplers[];

layout(push_constant) uniform PushConstants
{
    uint equirectangularMapId;
    uint cubeMapId;
    uint samplerId;

} pcs;

const float PI = 3.14159265359;

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
    vec3 cubeTC = getCubeMapTexCoord(vec2(imageSize(globalCubeMaps[pcs.cubeMapId])));

    float phi = atan(cubeTC.z, cubeTC.x);
    float theta = acos(cubeTC.y);
    vec2 uv = vec2(phi / (2.0 * PI) + 0.5, theta / PI);

    vec4 color = texture(sampler2D(globalTextures[pcs.equirectangularMapId], globalSamplers[pcs.samplerId]), uv);
    color = min(color, vec4(500.0));

    imageStore(globalCubeMaps[pcs.cubeMapId], ivec3(gl_GlobalInvocationID), color);
}