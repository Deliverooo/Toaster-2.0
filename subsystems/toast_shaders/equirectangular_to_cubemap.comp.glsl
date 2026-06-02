#version 460

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(set = 1, binding = 0, rgba16f) restrict uniform writeonly imageCube o_CubeMap;
layout(set = 1, binding = 1) uniform sampler2D u_EquirectangularMap;

const float PI = 3.14159265359f;

vec3 getCubeMapUV(vec2 p_image_size)
{
    vec2 st = gl_GlobalInvocationID.xy / p_image_size;
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y, uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);
    return normalize(ret);
}

void main()
{
    vec3 cube_uv = getCubeMapUV(vec2(imageSize(o_CubeMap)));

    float phi = atan(cube_uv.z, cube_uv.x);
    float theta = acos(cube_uv.y);
    vec2 uv = vec2(phi / (2.0 * PI) + 0.5, theta / PI);

    vec4 colour = texture(u_EquirectangularMap, uv);
    colour = min(colour, vec4(500.0));
    imageStore(o_CubeMap, ivec3(gl_GlobalInvocationID), colour);
}