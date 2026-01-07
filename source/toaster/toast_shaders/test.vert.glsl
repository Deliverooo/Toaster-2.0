#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

struct VertexOut
{
    vec3 fragPos;
    vec2 texCoord;
};

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out VertexOut o_vertex;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(a_Position, 1.0);
    o_vertex.fragPos = a_Position;
    o_vertex.texCoord = a_TexCoord;
}