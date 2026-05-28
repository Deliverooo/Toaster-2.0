#version 460

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 1, binding = 0) uniform samplerCube u_EnvironmentMap;

layout(set = 1, binding = 1, rgba16f) uniform writeonly imageCube o_Irradiance;

const float PI = 3.14159265359f;

vec3 getCubeMapDirection(vec2 p_uv, uint p_face)
{
    vec3 dir = vec3(0.0f);
    switch (p_face) {
        case 0: dir = vec3(1.0f, -p_uv.y, -p_uv.x); break;// +X
        case 1: dir = vec3(-1.0f, -p_uv.y, p_uv.x); break;// -X
        case 2: dir = vec3(p_uv.x, 1.0f, p_uv.y); break;// +Y
        case 3: dir = vec3(p_uv.x, -1.0f, -p_uv.y); break;// -Y
        case 4: dir = vec3(p_uv.x, -p_uv.y, 1.0f); break;// +Z
        case 5: dir = vec3(-p_uv.x, -p_uv.y, -1.0f); break;// -Z
    }
    return normalize(dir);
}

void main()
{
    ivec2 image_size = imageSize(o_Irradiance);
    ivec3 global_id = ivec3(gl_GlobalInvocationID);

    if (global_id.x >= image_size.x || global_id.y >= image_size.y) return;

    vec2 uv = (vec2(global_id.xy) + vec2(0.5f)) / vec2(image_size);
    uv = uv * 2.0f - 1.0f;

    vec3 normal = getCubeMapDirection(uv, global_id.z);

    vec3 irradiance = vec3(0.0f);
    vec3 up = abs(normal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sample_delta = 0.025f;
    float num_samples = 0.0f;

    for (float phi = 0.0f; phi < 2.0f * PI; phi += sample_delta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sample_delta)
        {
            vec3 tangent_sample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

            irradiance += texture(u_EnvironmentMap, sample_vec).rgb * cos(theta) * sin(theta);
            num_samples++;
        }
    }

    irradiance = PI * irradiance * (1.0f / num_samples);

    imageStore(o_Irradiance, global_id, vec4(irradiance, 1.0f));
}
