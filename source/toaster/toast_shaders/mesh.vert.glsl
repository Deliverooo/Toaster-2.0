#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 v_WorldPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoord;

void main()
{
    vec4 worldPos = ubo.model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    v_Normal = normalize(normalMatrix * a_Normal);
    
    v_TexCoord = a_TexCoord;
    
    gl_Position = ubo.proj * ubo.view * worldPos;
}
